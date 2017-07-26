#!/bin/bash -e
#
# Build Curl and OpenSSL
#
function usage() {
    echo "Usage: `basename $0` [-t <Release|Debug>]"
    echo "Builds Curl and OpenSSL"
    echo "-t <Release/Debug> : Release or Debug builds"
    exit 2
}
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OPENSSL_SOURCE_DIR=$DIR/../openssl-1.1.0f/
LIBCURL_SOURCE_DIR=$DIR/../curl-7.54.1/

target=Release
while getopts ":ht:s:" opt; do
  case $opt in
    t) target=$OPTARG ;;
    h) usage;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    :) echo "Option -$OPTARG requires an argument."; >&2 exit 1 ;;
  esac
done

[[ "$target" != "Debug" && "$target" != "Release" ]] && \
    echo "target must be either Debug/Release." && usage

echo "Options:"
echo "  target       = $target"
echo "PATH="$PATH

DEPENDENCY_LINUX=$DIR/../deps/linux
rm -rf $DEPENDENCY_LINUX
mkdir -p $DEPENDENCY_LINUX

# build openssl
OPENSSL_BUILD_DIR=$DEPENDENCY_LINUX/openssl 
openssl_config_opts=()
openssl_config_opts+=(
    "no-shared"
    "--prefix=$OPENSSL_BUILD_DIR"
)
if [[ "$target" != "Release" ]]; then
    openssl_config_opts+=("-d")
fi
cd $OPENSSL_SOURCE_DIR
./config ${openssl_config_opts[@]}
make depend
echo "Building and Installing OpenSSL"
make 2>&1 > /dev/null
make install_sw install_ssldirs 2>&1 > /dev/null

# build libcurl
curl_configure_opts=()
if [[ "$target" != "Release" ]]; then
    curl_configure_opts+=("--enable-debug")
fi
LIBCURL_BUILD_DIR=$DEPENDENCY_LINUX/curl
curl_configure_opts+=(
    "--with-ssl=$OPENSSL_BUILD_DIR"
    "--without-nss"
)
rm -rf $LIBCURL_BUILD_DIR
curl_configure_opts+=(
    "--enable-static"
    "--disable-shared"
    "--prefix=$LIBCURL_BUILD_DIR"
    "--without-libssh2"
    "--disable-rtsp"
    "--disable-ldap"
    "--disable-ldaps"
    "--disable-telnet"
    "--disable-tftp"
    "--disable-imap"
    "--disable-smb"
    "--disable-smtp"
    "--disable-gopher"
    "--disable-pop3"
    "--disable-ftp"
    "--disable-dict"
    "--disable-file"
    "--disable-manual"
)
cd $LIBCURL_SOURCE_DIR
echo "Building Curl with OpenSSL"
PKG_CONFIG="pkg-config -static" LIBS="-ldl" ./configure ${curl_configure_opts[@]}
echo "Building and Installing Curl"
make 2>&1 > /dev/null
make install 2>&1 > /dev/null
