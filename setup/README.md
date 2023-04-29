# Setup files for uNav CAN Interface

This folder contains the files required to setup the gs_usb CAN interface for the uNav board.

## 99-unav.rules

This file setup the UDev rules to recognize uNav as a gs_usb adapter. This is obtained by adding
vid/pid to the `newid` for the gs_usb usb module.

## 80-can.network

This file works in conjunction with the `systemd-network` daemon to auto-init the CAN* interface
once the usb module is initialised.

## Setup

just run the `install.sh` file as root to setup all required configurations.
In case of errors double check that [udev](https://manpages.ubuntu.com/manpages/focal/man7/udev.7.html)
and [systemd-network](https://manpages.ubuntu.com/manpages/focal/man8/systemd-networkd.service.8.html)
are correctly installed and running.
