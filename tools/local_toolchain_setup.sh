#!/usr/bin/env bash
set -ex

VERSION="12.3.rel1"
PLATFORM=
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_PATH="$(dirname "${SCRIPT_PATH}")"
ARCH="$(uname -m)"

TOOL_ARCH="$(uname -m)_$(uname)"

TOOLS_FOLDER="${SCRIPT_PATH}/../.tools/${TOOL_ARCH}"
TOOLS_INSTALL_FOLDER="${TOOLS_INSTALL_FOLDER:=$TOOLS_FOLDER}"
mkdir -p "${TOOLS_FOLDER}"
mkdir -p "${TOOLS_INSTALL_FOLDER}"

case "$(uname)" in
    Darwin)
        PLATFORM="darwin-"
        ARCH=${ARCH/aarch64/arm64}
esac


GCC_BASE_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${VERSION}/binrel/"
GCC_BINARY="arm-gnu-toolchain-${VERSION}-${PLATFORM}${ARCH}-arm-none-eabi.tar.xz"
GCC_CHECKSUM="arm-gnu-toolchain-${VERSION}-${PLATFORM}${ARCH}-arm-none-eabi.tar.xz.sha256asc"
TMP_DIR=$(mktemp -d -t arm-none-eabi-XXXXXXXXXX)

cd "${TMP_DIR}"

curl -SL "${GCC_BASE_URL}/${GCC_BINARY}" -O
curl -SL "${GCC_BASE_URL}/${GCC_CHECKSUM}" -O
sha256sum -c --ignore-missing "${GCC_CHECKSUM}"
tar -xf "${GCC_BINARY}"
rm "${GCC_BINARY}" "${GCC_CHECKSUM}"

mv "$(ls)" "${TOOLS_INSTALL_FOLDER}/arm-gnu-toolchain-${VERSION}"
ln -s "${TOOLS_INSTALL_FOLDER}/arm-gnu-toolchain-${VERSION}" "${TOOLS_INSTALL_FOLDER}/arm-gnu-toolchain"

echo "GCC_PATH=${TOOLS_INSTALL_FOLDER}/arm-gnu-toolchain/bin" > "${TOOLS_FOLDER}/.env"
