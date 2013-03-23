#! /bin/bash
set -e

mkdir -p build

if [ -f "/usr/bin/ninja" ] ; then
  EXTRA_ARGS="-G Ninja"
  BUILD_COMMAND="ninja"
else
  BUILD_COMMAND="make"
fi

echo "Using $BUILD_COMMAND to build"
(
  cd build
  cmake .. $EXTRA_ARGS -DCMAKE_INSTALL_PREFIX=../../install -Dlocal_install=ON
  $BUILD_COMMAND
)
