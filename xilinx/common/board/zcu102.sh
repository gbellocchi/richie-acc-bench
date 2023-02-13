#!/usr/bin/env bash

# Read input arguments.
readonly THIS_DIR="$1"
readonly FPGA_HW_DIR="$2"
readonly PROJECT_NAME="$3"
readonly DESIGN_NAME="$4"
readonly BOARD_DIR="$5"

# Local config location.
if $UNIMORE; then
  readonly LOCAL_CFG="$BOARD_DIR/../configfile/unimore.cfg"
else
  if [ -z "$IIS" ]; then
    readonly LOCAL_CFG="$BOARD_DIR/../configfile/iis.cfg"
  fi
fi

set -e

# Change working directory to path of script, so this script can be executed from anywhere.
cd "$THIS_DIR"
# Resolve symlinks.
cd "$(pwd -P)"

# Initialize Python environment suitable for PetaLinux.
python3.6 -m venv .venv
ln -sf python3.6 .venv/bin/python3
source .venv/bin/activate

if [ -n "$NO_IIS" ]; then
  PETALINUX_VER=''
else
  if [ -z "$PETALINUX_VER" ]; then
    PETALINUX_VER="vitis-2019.2"
  fi
fi
readonly PETALINUX_VER
readonly TARGET=zcu102

# Create project.
if [ ! -d "$TARGET" ]; then
    $PETALINUX_VER petalinux-create -t project -n "$TARGET" --template zynqMP
fi
cd "$TARGET"

# Initialize and set necessary configuration from config and local config.
$PETALINUX_VER petalinux-config --oldconfig --get-hw-description "$FPGA_HW_DIR"

mkdir -p components/ext_sources
cd components/ext_sources
if [ ! -d "linux-xlnx" ]; then
    git clone --depth 1 --single-branch --branch xilinx-v2019.2.01 git://github.com/Xilinx/linux-xlnx.git
fi
cd linux-xlnx
git checkout tags/xilinx-v2019.2.01

cd ../../../
sed -i 's|CONFIG_SUBSYSTEM_COMPONENT_LINUX__KERNEL_NAME_LINUX__XLNX=y||' project-spec/configs/config
echo 'CONFIG_SUBSYSTEM_COMPONENT_LINUX__KERNEL_NAME_EXT__LOCAL__SRC=y' >> project-spec/configs/config
echo 'CONFIG_SUBSYSTEM_COMPONENT_LINUX__KERNEL_NAME_EXT_LOCAL_SRC_PATH="${TOPDIR}/../components/ext_sources/linux-xlnx"' >> project-spec/configs/config
echo 'CONFIG_SUBSYSTEM_MACHINE_NAME="zcu102-revb"' >> project-spec/configs/config

# Image Packaging Configuration
if [ -f "$LOCAL_CFG" ] && grep -q PT_ROOTFS_NFS "$LOCAL_CFG"; then
  sed -i 's|CONFIG_SUBSYSTEM_ROOTFS_INITRAMFS=y||' project-spec/configs/config
  sed -i 's|CONFIG_SUBSYSTEM_ROOTFS_INITRD=y||' project-spec/configs/config
  sed -i 's|CONFIG_SUBSYSTEM_ROOTFS_JFFS2=y||' project-spec/configs/config
  sed -i 's|CONFIG_SUBSYSTEM_ROOTFS_SD=y||' project-spec/configs/config
  sed -i 's|CONFIG_SUBSYSTEM_ROOTFS_OTHER=y||' project-spec/configs/config
  echo 'CONFIG_SUBSYSTEM_ROOTFS_NFS=y' >> project-spec/configs/config
  sed -e 's/PT_NFSROOT_DIR/CONFIG_SUBSYSTEM_NFSROOT_DIR/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
  sed -e 's/PT_NFSSERVER_IP/CONFIG_SUBSYSTEM_NFSSERVER_IP/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
else
  echo 'CONFIG_SUBSYSTEM_ROOTFS_SD=y' >> project-spec/configs/config
  echo 'CONFIG_SUBSYSTEM_SDROOT_DEV="/dev/mmcblk0p2"' >> project-spec/configs/config
fi

# Ethernet Settings
if [ -f "$LOCAL_CFG" ] && grep -q PT_ETH_MAC "$LOCAL_CFG"; then
    sed -e 's/PT_ETH_MAC/CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_MAC/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
fi
sed -i 's|CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_USE_DHCP=y||' project-spec/configs/config
if [ -f "$LOCAL_CFG" ] && grep -q IP_ADDR "$LOCAL_CFG"; then
  echo '# CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_USE_DHCP is not set' >> project-spec/configs/config
  sed -e 's/IP_ADDR/CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_IP_ADDRESS/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
  sed -e 's/NETMASK/CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_IP_NETMASK/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
  sed -e 's/GATEWAY/CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_IP_GATEWAY/;t;d' "$LOCAL_CFG" >> project-spec/configs/config
else
  echo 'CONFIG_SUBSYSTEM_ETHERNET_PSU_ETHERNET_3_USE_DHCP=y' >> project-spec/configs/config
fi

$PETALINUX_VER petalinux-config --oldconfig --get-hw-description "$FPGA_HW_DIR"

echo "
/include/ \"system-conf.dtsi\"
/include/ \"${BOARD_DIR}/dtsi/${DESIGN_NAME}.dtsi\"
/ {
};
" > project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi

# Configure RootFS.
rootfs_enable() {
    sed -i -e "s/# CONFIG_$1 is not set/CONFIG_$1=y/" project-spec/configs/rootfs_config
}
for pkg in \
    bash \
    bash-completion \
    bc \
    ed \
    grep \
    patch \
    sed \
    util-linux \
    util-linux-blkid \
    util-linux-lscpu \
    vim \
; do
  rootfs_enable $pkg
done

# Build PetaLinux.
set +e
$PETALINUX_VER petalinux-build
echo "First build might fail, this is expected..."
set -e
mkdir -p build/tmp/work/aarch64-xilinx-linux/external-hdf/1.0-r0/git/plnx_aarch64/

if [ -n "$NO_IIS" ]; then
  cp project-spec/hw-description/system.xsa build/tmp/work/aarch64-xilinx-linux/external-hdf/1.0-r0/git/plnx_aarch64/
else
  cp project-spec/hw-description/system.hdf build/tmp/work/aarch64-xilinx-linux/external-hdf/1.0-r0/git/plnx_aarch64/
fi

$PETALINUX_VER petalinux-build

mkdir -p build/tmp/work/aarch64-xilinx-linux/external-hdf/1.0-r0/git/plnx_aarch64/

# Generate images including bitstream with `petalinux-package`.
cp "$bitstream" ${PROJECT_NAME}.bit
echo "
the_ROM_image:
{
  [init] regs.init
  [bootloader] zynqmp_fsbl.elf
  [pmufw_image] pmufw.elf
  [destination_device=pl] ${PROJECT_NAME}.bit
  [destination_cpu=a53-0, exception_level=el-3, trustzone] bl31.elf
  [destination_cpu=a53-0, exception_level=el-2] u-boot.elf
}
" > bootgen.bif
$PETALINUX_VER petalinux-package --boot --force \
  --fsbl zynqmp_fsbl.elf \
  --fpga ${PROJECT_NAME}.bit \
  --u-boot u-boot.elf \
  --pmufw pmufw.elf \
  --bif bootgen.bif

# Copy built files to output directory.
cp -rf images/linux/* ${THIS_DIR}/output