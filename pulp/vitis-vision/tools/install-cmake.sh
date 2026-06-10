#!/bin/bash

if [[ -z "$WKS_VITIS" ]]; then
  echo "Source the Vitis setup first."
  exit 1
fi

# Create install directory
mkdir -p $install_path
cd $install_path

# cmake version
version=3.25
build=1

# don't modify from here
limit=3.20
result=$(echo "$version >= $limit" | bc -l)
os=$([ "$result" == 1 ] && echo "linux" || echo "Linux")

# Download
wget https://cmake.org/files/v$version/cmake-$version.$build-$os-x86_64.sh
sudo mkdir /opt/cmake
sudo sh cmake-$version.$build-$os-x86_64.sh --prefix=/opt/cmake

sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
