PATH=$PATH:/opt/newcross/mipsel-unknown-linux-gnu/bin/
export PREFIX=/opt/newcross/mipsel-unknown-linux-gnu/mipsel-unknown-linux-gnu/sys-root/usr
export HOST=mipsel-unknown-linux-gnu
export CC=$CCPATH$HOST-gcc
export CCPATH=/opt/newcross/mipsel-unknown-linux-gnu/bin/
export AR=$CCPATH$HOST-ar
export NM=$CCPATH$HOST-nm
export RANLIB=$CCPATH$HOST-ranlib
export OBJDUMP=$CCPATH$HOST-objdump
export STRIP=$CCPATH$HOST-strip
export CFLAGS="-Wall -g0 -O2 \
	-D__KERNEL_STRICT_NAMES  \
	-I"$PWD"/include -I"$PWD"/include/linux/dvb -I"$PREFIX"/include/freetype2"
#-DUSE_NEVIS_GXA
export CXXFLAGS="-Wall -g0 -O2 \
	-D__KERNEL_STRICT_NAMES  \
	-I"$PWD"/include -I"$PWD"/include/linux/dvb -I"$PREFIX"/include/freetype2"
#-DUSE_NEVIS_GXA
export LDFLAGS="-L"$PREFIX"/lib -lcurl -lssl -lcrypto -ldl"
export DVB_API_VERSION=3
export FREETYPE_CONFIG=$PREFIX/bin/freetype-config
export CURL_CONFIG=$PREFIX/bin/curl-config
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes
echo "AZBOX Neutrino ENV gesetzt"
