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

echo "Resetting board."

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

# Print informations
echo "Resetting board at ${hw_server_ip}:${hw_server_port}"

# Send Reset Commands through XSDB
cat <<EOF | vivado-2019.1.1 xsdb
connect -host ${hw_server_ip} -port ${hw_server_port}

puts -nonewline "Reseting"
targets -set -nocase -filter {name =~ "*PSU*"}
stop
rst -system
after 2000
targets -set -nocase -filter {name =~ "*PMU*"}
stop
rst -system
after 2000
targets -set -nocase -filter {name =~ "*PSU*"}
stop
rst -system
after 2000
mwr 0xFFCA0038 0x1ff
targets -set -nocase -filter {name =~ "*MicroBlaze PMU*"}
dow "${THIS_DIR}/output/pmufw.elf"
after 2000
con
exit
EOF

# That's all folks!!
