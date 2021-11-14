#!/bin/sh
# you can either set the environment variables AUTOCONF, AUTOHEADER, AUTOMAKE,
# ACLOCAL, AUTOPOINT and/or LIBTOOLIZE to the right versions, or leave them
# unset and get the defaults

if test ! -f deps/cogl/autogen.sh;
then
  echo "+ Setting up COGL submodule"
  git submodule update --init deps/cogl || exit 1
fi

if test ! -f deps/meson-based/glib/meson.build;
then
  echo "+ Setting up GLIB submodule"
  git submodule update --init deps/meson-based/glib || exit 1
fi

cd deps/cogl
echo "patching cogl to build on Ubuntu focal.."
git apply ../../patches/0001-fix-build-on-Ubuntu-focal.patch 2>/dev/null && echo "ok"
NOCONFIGURE=1 ./autogen.sh
cd ../..

autoreconf --force --install || {
 echo 'autogen.sh failed';
 exit 1;
}

./configure --prefix=$(pwd)/build $@ || {
 echo 'configure failed';
 exit 1;
}

cp jihn .git/hooks/pre-commit
echo "BACK=\$PWD && cd $(pwd)/bombolla && make check && cd ../tests && make check && cd \$BACK" >> .git/hooks/pre-commit

echo
echo "Now type 'make' to compile this module."
echo
