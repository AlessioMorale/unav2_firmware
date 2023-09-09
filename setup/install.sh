#!/bin/bash
set -ex
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_PATH="$(dirname "${SCRIPT_PATH}")"

# Add the rules to recognize uNav as a gs_usb CAN interface
cp "${SCRIPT_PATH}/99-unav.rules" /etc/udev/rules.d/
udevadm control --reload

cp "${SCRIPT_PATH}/can_modules.conf" /etc/modules-load.d/
systemctl restart systemd-modules-load.service

cp "${SCRIPT_PATH}/bridge_can_device.service" /etc/systemd/system/
systemctl enable bridge_can_device
systemctl start bridge_can_device

# install the network configuration to automatically start the CAN interface
cp "${SCRIPT_PATH}/70-gs_usb_can.link" /etc/systemd/network/
cp "${SCRIPT_PATH}/80-can.network" /etc/systemd/network/
cp "${SCRIPT_PATH}/vcan0.netdev" /etc/systemd/network/
systemctl restart systemd-networkd.service
systemctl restart NetworkManager.service
