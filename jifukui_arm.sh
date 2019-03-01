#!/bin/sh
NATIVE_TOOLS_DIR=~/native-tools
NATIVE_TOOLS_SRC=$NATIVE_TOOLS_DIR/src
NATIVE_TOOLS_BLD=$NATIVE_TOOLS_DIR/build
NATIVE_GCC=$NATIVE_TOOLS_DIR/native-gcc
NATIVE_MINGW_W64_GCC=$NATIVE_TOOLS_DIR/native-mingw-w64-gcc

#Get upstream source packages.
rm -rf $NATIVE_TOOLS_SRC
mkdir -p $NATIVE_TOOLS_SRC
pushd $NATIVE_TOOLS_SRC
wget -c http://ftp.gnu.org/gnu/binutils/binutils-2.23.2.tar.gz
wget -c ftp://ftp.gnu.org/gnu/gcc/gcc-4.6.4/gcc-4.6.4.tar.bz2
wget -c ftp://gcc.gnu.org/pub/gcc/infrastructure/gmp-4.3.2.tar.bz2
wget -c ftp://gcc.gnu.org/pub/gcc/infrastructure/mpfr-2.4.2.tar.bz2
wget -c ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-0.8.1.tar.gz

wget -c http://downloads.sourceforge.net/project/mingw-w64/mingw-w64/\
mingw-w64-release/mingw-w64-v3.1.0.tar.bz2

find . -maxdepth 1 -name "*.tar.*" -exec tar xf ’{}’ \;
cd gcc-4.6.4
ln -s ../gmp-4.3.2 gmp
ln -s ../mpfr-2.4.2 mpfr
ln -s ../mpc-0.8.1 mpc
popd

#Build native gcc
mkdir -p $NATIVE_TOOLS_BLD/native-gcc
pushd $NATIVE_TOOLS_BLD/native-gcc
$NATIVE_TOOLS_SRC/gcc-4.6.4/configure \
--build=i686-linux-gnu \
--host=i686-linux-gnu \
--target=i686-linux-gnu \
--enable-languages=c,c++ \
--enable-shared \
--enable-threads=posix \
--disable-decimal-float \
--disable-libffi \
--disable-libgomp \
--disable-libmudflap \
--disable-libssp \
--disable-libstdcxx-pch \
--disable-multilib \
--disable-bootstrap \
--disable-nls \
--with-gnu-as \
--with-gnu-ld \
--enable-libstdcxx-debug \
--enable-targets=all \
--enable-checking=release \
--prefix=$NATIVE_TOOLS_DIR/native-gcc \
--with-host-libstdcxx="-static-libgcc \
-L /usr/lib/gcc/i486-linuxgnu/4.3.2/ -lstdc++ -lsupc++ -lm"

make && make install
popd
export PATH=$NATIVE_GCC/bin:$PATH
echo "export PATH=$NATIVE_GCC/bin:\$PATH" >> ~/.bashrc

#If no intention to generate ARM embedded toolchain for Windows platform,
#then following steps are unnecessary and we can quit now.
#Start to build native MinGW from building MinGW Binutils.

mkdir -p $NATIVE_TOOLS_BLD/binutils
pushd $NATIVE_TOOLS_BLD/binutils
$NATIVE_TOOLS_SRC/binutils-2.23.2/configure \
--target=i686-w64-mingw32 --disable-multilib \
--prefix=$NATIVE_MINGW_W64_GCC
make
make install
popd
export PATH=$NATIVE_MINGW_W64_GCC/bin:$PATH
mkdir -p $NATIVE_TOOLS_BLD/mingw-w64-header
pushd $NATIVE_TOOLS_BLD/mingw-w64-header
$NATIVE_TOOLS_SRC/mingw-w64-v3.1.0/mingw-w64-headers/configure \
--host=i686-w64-mingw32 \
--prefix=$NATIVE_MINGW_W64_GCC/i686-w64-mingw32

make install
pushd $NATIVE_MINGW_W64_GCC
ln -s i686-w64-mingw32 mingw32
popd
popd
mkdir -p $NATIVE_TOOLS_BLD/mingw-gcc
pushd $NATIVE_TOOLS_BLD/mingw-gcc
$NATIVE_TOOLS_SRC/gcc-4.6.4/configure \
--target=i686-w64-mingw32 \
--prefix=$NATIVE_MINGW_W64_GCC \
--disable-multilib --enable-languages=c,c++
make all-gcc
make install-gcc
popd
mkdir -p $NATIVE_TOOLS_BLD/mingw-w64-crt
pushd $NATIVE_TOOLS_BLD/mingw-w64-crt
$NATIVE_TOOLS_SRC/mingw-w64-v3.1.0/mingw-w64-crt/configure \
--host=i686-w64-mingw32 \
--prefix=$NATIVE_MINGW_W64_GCC/i686-w64-mingw32
make
make install
popd
pushd $NATIVE_TOOLS_BLD/mingw-gcc
make
make install
popd
#Get and deploy the InstallJammer
pushd $NATIVE_TOOLS_SRC
wget -c http://downloads.sourceforge.net/project/installjammer/\
InstallJammer/1.2.15/installjammer-1.2.15.tar.gz
pushd $NATIVE_TOOLS_DIR
tar xf $NATIVE_TOOLS_SRC/installjammer-1.2.15.tar.gz
popd
popd
#After those two steps, remember to re-source the .bashrc to make
#those new native tools effective before build ARM embedded toolchain.
#There is no need to source .bashrc every time because the reboot or
#re-login will source .bashrc automatically.
echo "export PATH=$NATIVE_MINGW_W64_GCC/bin:\$PATH" >> ~/.bashrc
echo "export PATH=$NATIVE_TOOLS_DIR/installjammer:\$PATH" >> ~/.bashrc
