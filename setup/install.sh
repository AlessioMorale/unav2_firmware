#!/bin/bash
set -ex
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_PATH="$(dirname "${SCRIPT_PATH}")"

# Add the rules to recognize uNav as a gs_usb CAN interface
cp "${SCRIPT_PATH}/99-unav.rules" /etc/udev/rules.d/
udevadm control --reload

# install the network configuration to automatically start the CAN interface
cp "${SCRIPT_PATH}/80-can.network" /etc/systemd/network/
systemctl restart systemd-networkd.service
