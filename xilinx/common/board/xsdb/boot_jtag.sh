#!/usr/bin/env bash

# Read input arguments.
readonly THIS_DIR="$1"
readonly BOARD_DIR="$2"
readonly FPGA_HW_DIR="$3"
readonly PROJECT_NAME="$4"
readonly DESIGN_NAME="$5"
readonly BOARD_MODEL="$6"

echo "Booting $BOARD_MODEL board with design "$DESIGN_NAME"."

# Local config location.
if $UNIMORE; then
  CONFIG_FILE=$BOARD_DIR/../configfile/unimore.cfg
else
  if [ -z "$IIS" ]; then
    CONFIG_FILE=$BOARD_DIR/../configfile/iis.cfg
  fi
fi

# Output directory.
OUTPUT_DIR=$THIS_DIR/output

# HW Server IP Configuration.
if [ -f ${CONFIG_FILE} ] && grep -q HW_SERVER_IP ${CONFIG_FILE}; then
  eval HW_SERVER_IP=$(grep HW_SERVER_IP ${CONFIG_FILE} | sed 's/.*=//' | tr -d '"')
  echo "Hardware server IP: ${HW_SERVER_IP}."
  hw_server_ip=${HW_SERVER_IP}
else
  echo "Hardware server IP: 127.0.0.1 (Default)."
  hw_server_ip=127.0.0.1
fi

# HW Server Port Configuration.
if [ -f ${CONFIG_FILE} ] && grep -q HW_SERVER_PORT ${CONFIG_FILE}; then
  eval HW_SERVER_PORT=$(grep HW_SERVER_PORT ${CONFIG_FILE} | sed 's/.*=//' | tr -d '"')
  echo "Hardware server IP: ${HW_SERVER_PORT}."
  hw_server_port=${HW_SERVER_PORT}
else
  echo "Hardware server port: 3121 (Default)."
  hw_server_port=3121
fi

# Get petalinux version.
if $NO_IIS; then
  PETALINUX_VER=''
else
  if [ -z "$PETALINUX_VER" ]; then
    PETALINUX_VER="vitis-2019.2"
  fi
fi

# Reset the board (this is required to clean up the system)
${BOARD_DIR}/xsdb/reset_jtag.sh ${THIS_DIR} ${BOARD_DIR}

# Boot Design from JTAG
if [ -z "${FPGA_HW_DIR}/${BOARD_MODEL}.bit" ]; then
    echo "ERROR: please build your hardware design before doing this."
else
    petalinux_prj=${THIS_DIR}/${BOARD_MODEL}
    bitstream=${FPGA_HW_DIR}/${DESIGN_NAME}.bit
    cd ${petalinux_prj}
    echo "Booting Petalinux project: ${petalinux_prj}"
    echo "HW Server: ${hw_server_ip}:${hw_server_port} Bitstream: ${bitstream}"
    $PETALINUX_VER petalinux-boot --jtag --u-boot --fpga --bitstream ${bitstream} -v --hw_server-url ${hw_server_ip}:${hw_server_port}
fi

# That's all folks!!
