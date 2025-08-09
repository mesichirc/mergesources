#/bin/sh

CFLAGS="-Wall -Wextra -Wpedantic -Werror"
LDFLAGS=""
SRC="debugger.c"
BUNDLE="./app"
PROJECT="./debugger"

case $1 in 
  'build')
    case $2 in
      '')
      rm -fr $BUNDLE
      mkdir $BUNDLE
      clang -o "$BUNDLE/$PROJECT" -g -O0 $CFLAGS $LDFLAGS $SRC
      ;;
    esac 
    ;;
  debug)
    case $2 in
      '')
        lldb -n $BUNDLE/$PROJECT
        ;;
      wait)
        lldb -n $BUNDLE/$PROJECT-w
        ;;
    esac
    ;;
  clean)
		rm -fr $BUNDLE
    ;; 
esac

