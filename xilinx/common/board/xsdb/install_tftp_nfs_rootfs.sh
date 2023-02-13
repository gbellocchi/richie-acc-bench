#!/usr/bin/env bash

# Read input arguments.
readonly THIS_DIR="$1"
readonly BOARD_DIR="$2"

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

# Install tftpboot.
if [ -f ${CONFIG_FILE} ] && grep -q PT_TFTPBOOT_DIR ${CONFIG_FILE}; then
    eval TFTPBOOT_DIR=$(grep PT_TFTPBOOT_DIR ${CONFIG_FILE} | sed 's/.*=//' | tr -d '"')
    echo "Installing Boot Files -> $TFTPBOOT_DIR"
    cp ${OUTPUT_DIR}/Image $TFTPBOOT_DIR
    cp ${OUTPUT_DIR}/system.dtb $TFTPBOOT_DIR
else
    echo "Installing Boot Files: SKIPPED (PT_TFTPBOOT_DIR is not set in local.cfg)"
fi

# Install rootfs.
if [ -f ${CONFIG_FILE} ] && grep -q PT_NFSROOT_DIR ${CONFIG_FILE}; then
    eval NFSROOT_DIR=$(grep PT_NFSROOT_DIR ${CONFIG_FILE} | sed 's/.*=//' | tr -d '"')
    echo "Installing Host RootFS -> $NFSROOT_DIR"
    sudo tar -xf ${OUTPUT_DIR}/rootfs.tar.gz -C $NFSROOT_DIR
else
    echo "Installing RootFS: SKIPPED (PT_NFSROOT_DIR is not set in local.cfg)"
fi