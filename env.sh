PATH=$PATH:/opt/newcross/mipsel-unknown-linux-gnu/bin/
export PREFIX=/opt/newcross/mipsel-unknown-linux-gnu/mipsel-unknown-linux-gnu/sys-root/usr
export HOST=mipsel-unknown-linux-gnu
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
export CC=$CCPATH$HOST-gcc
export CCPATH=/opt/newcross/mipsel-unknown-linux-gnu/bin/
export AR=$CCPATH$HOST-ar
export NM=$CCPATH$HOST-nm
export RANLIB=$CCPATH$HOST-ranlib
export OBJDUMP=$CCPATH$HOST-objdump
export STRIP=$CCPATH$HOST-strip
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes
echo "AZBOX ENV gesetzt"
