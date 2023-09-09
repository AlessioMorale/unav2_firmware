#!/usr/bin/env bash
set -ex

architecture=$(dpkg --print-architecture)
case "${architecture}" in
    arm64)
        ARCH=aarch64 ;;
    amd64)
        ARCH=x86_64 ;;
    *)
        echo "Unsupported architecture ${architecture}."
        exit 1
        ;;
esac
GCC_BASE_URL="https://developer.arm.com/-/media/Files/downloads/gnu/12.3.rel1/binrel/"
GCC_BINARY="arm-gnu-toolchain-12.3.rel1-${ARCH}-arm-none-eabi.tar.xz"
GCC_CHECKSUM="arm-gnu-toolchain-12.3.rel1-${ARCH}-arm-none-eabi.tar.xz.sha256asc"
TMP_DIR=$(mktemp -d -t arm-none-eabi-XXXXXXXXXX)

cd "${TMP_DIR}"

curl -sSL "${GCC_BASE_URL}/${GCC_BINARY}" -O
curl -sSL "${GCC_BASE_URL}/${GCC_CHECKSUM}" -O
sha256sum -c --ignore-missing "${GCC_CHECKSUM}"
tar -xf "${GCC_BINARY}"
rm ${GCC_BINARY} ${GCC_CHECKSUM}

mv "$(ls)" /opt/arm-gnu-toolchain-12.3
ln -s /opt/arm-gnu-toolchain-12.3 /opt/arm-gnu-toolchain