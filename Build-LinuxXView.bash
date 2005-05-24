# This script sets up the necessary environment to build XView for Linux.
# It should be sourced by bash or run with one of the arguments shown below.
# If you want to use a installation directory prefix different from /, set
# $INSTPREFIX accordingly. Setting $OPENWINHOME to values != /usr/openwin
# is untested at the moment!
# This script relies on wrappers for make and imake being in the current
# directory. The imake-wrapper appends the file $IMAKEAPPEND to the created
# Makefile if it exists. The make-wrapper simply calls pmake instead of make.
# Don't try to build XView with bash 2.0 installed as /bin/sh -- it will fail
# (due to a bug in X11R6's Imake.rules). Use bash 1.14 or 2.01 instead.

if ! [ -d lib/libxview ]; then
  echo Please change to XView source tree and try again
  exit 1
fi

# Set the directories that will be used for installation
[ -z "$OPENWINHOME" ] && OPENWINHOME=/usr/openwin
[ -z "$X11DIR" ] && X11DIR=/usr/X11R6
OWDEST=$INSTPREFIX$OPENWINHOME
X11DEST=$INSTPREFIX$X11DIR/lib/X11/config

# Generate file that gets appended to every Makefile created by the imake-wrapper

if [ -n "$BUILDPREFIX" ]; then
  IMAKE_EXTRA_INCLUDES="-I$BUILDPREFIX/usr/X11R6/include -I$BUILDPREFIX/usr/include"
  IMAKE_EXTRA_LOCAL_LDFLAGS="-L$BUILDPREFIX/usr/X11R6/lib -L$BUILDPREFIX/usr/lib"
fi
cat > imake.append <<EOF

# Variable-definitions appended by imake-wrapper
  XVDESTDIR      = $OWDEST
  EXTRA_DEFINES  = -DOPENWINHOME_DEFAULT=\"$OPENWINHOME\"
  CONFIGDIR      = $X11DEST
  INCLUDES      ?=
  INCLUDES      := -I`pwd`/build/include -I$OWDEST/include $IMAKE_EXTRA_INCLUDES \$(INCLUDES)
  LOCAL_LDFLAGS ?=
  LOCAL_LDFLAGS := -L$OWDEST/lib $IMAKE_EXTRA_LOCAL_LDFLAGS \$(LOCAL_LDFLAGS)
# End of variable-definitions appended by imake-wrapper

EOF
#INCLUDES      := -I`pwd`/build/include -I$OWDEST/include $IMAKE_EXTRA_INCLUDES -I/usr/X11R6/include \$(INCLUDES)
#LOCAL_LDFLAGS := -L$OWDEST/lib $IMAKE_EXTRA_LOCAL_LDFLAGS -L/usr/X11R6/lib \$(LOCAL_LDFLAGS)

IMAKEAPPEND="`pwd`/imake.append"
IMAKEINCLUDE="-I`pwd`/config -I$BUILDPREFIX/usr/X11R6/lib/X11/config"

# Make sure our wrappers are in the path
if [ -z "$XVIEW_SETUP" ]; then
  [ -n "$BUILDPREFIX" ] && PATH="$BUILDPREFIX/bin:$PATH"
  PATH="`pwd`:$PATH"
fi

PS1='\h:\w(XView-build)\$ '
XVIEW_SETUP=1
export OPENWINHOME X11DIR OWDEST X11DEST IMAKEINCLUDE IMAKEAPPEND \
       PATH PS1 XVIEW_SETUP
hash -r

# patch doesn't restore the permissions, so make sure our wrappers are
# executable
chmod a+rx imake make

if [ "x$1" = xall ]; then
  set -- libs instlibs clients contrib olvwm instclients instcontrib instolvwm instfinish
fi

while [ $# -gt 0 ]; do
  case "$1" in
    libs)
      cd config
      imake -DUseInstalled $IMAKEINCLUDE
      cd ..
      imake -DUseInstalled $IMAKEINCLUDE
      make World
      ;;
    instlibs)
      install -d $OWDEST $X11DEST
      make install install.man
      make SUBDIRS=doc install
      ;;
    clients)
      make Clients
      ;;

    contrib)
      make Contrib
      ;;
    olvwm)
      (
        cd clients/olvwm-4.1
        imake -DUseInstalled $IMAKEINCLUDE
        make depend
        make
      )
      ;;
    instclients)
      make SUBDIRS=clients install install.man install.srcs
      ;;
    instcontrib)
      make SUBDIRS=contrib install install.man install.srcs
      ;;
    instolvwm)
      (
        cd clients/olvwm-4.1 && make install install.man
      )
      ;;
    instfinish)
      [ -e $OWDEST/lib/openwin-menu-std ] || mv $OWDEST/lib/openwin-menu $OWDEST/lib/openwin-menu-std
      install -d $OWDEST/lib/xview $OWDEST/share/locale/C/props
      install -m 644 contrib/misc/openwin-menu* $OWDEST/lib
      install -m 755 contrib/misc/Xinitrc $OWDEST/lib
      install -m 755 contrib/misc/{openwin,owplaces} $OWDEST/bin
      install -m 644 contrib/misc/props-locale.C $OWDEST/share/locale/C/props/C
      install -m 644 contrib/misc/props-locale.basic_setting $OWDEST/share/locale/C/props/basic_setting
      [ -e  $OWDEST/bin/full1.sed ] && \
        mv $OWDEST/bin/{full[12],minimal[1234]}.sed $OWDEST/lib/xview
      chmod -R a+rX,u+w,go-w $OWDEST $X11DEST/XView.*
      chmod a+x $OWDEST/share/src/xview/examples/bin/*
      chown -R root.root $OWDEST $X11DEST/XView.*
      [ -z "$INSTPREFIX" ] || chown -R root.root $INSTPREFIX/usr
      ;;
    tar)
      DIRNAME="`pwd`"
      DIRNAME="${DIRNAME##*/}"
      ( cd $INSTPREFIX && tar cv ${OPENWINHOME##/} ${X11DIR##/}/lib/X11/config/XView.*) | \
        gzip -9 > ../$DIRNAME.bin.tar.gz
      ;;
    srctar)
      rm -f imake.append
      DIRNAME="`pwd`"
      DIRNAME="${DIRNAME##*/}"
      (
        cd ..
        chmod -R a+rX,u+w,go-w "$DIRNAME"
        tar cv "$DIRNAME" | gzip -9 > $DIRNAME.src.tar.gz
      )
      ;;
    clean)
      rm -f imake.append
      [ -e clients/olvwm-4.1/Makefile ] && (cd clients/olvwm-4.1 && make clean)
      [ -e Makefile ] && make Clean
      find -name Makefile -o -name "*~" -o -name "*.so.*" -o -name "*.so" | xargs -r rm
      ;;
    diff)
      rm -f imake.append
      DIRNAME="`pwd`"
      DIRNAME="${DIRNAME##*/}"
      (
        cd ..
        diff --context --recursive --new-file xview3.2p1-X11R6 $DIRNAME | gzip -9 > $DIRNAME.diff.gz
      )
      ;;
  esac
  shift
done
