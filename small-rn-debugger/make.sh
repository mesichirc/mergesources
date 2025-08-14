#/bin/sh

CFLAGS="-Wall -Wextra -Wpedantic -Werror"
LDFLAGS=""
SRC="debugger.c"
BUNDLE="./app"
PROJECT="./debugger"
OS="$(uname)"

case $OS in
  'Darwin')
    LDFLAGS="-framework CoreVideo -framework Cocoa -framework OpenGL -framework IOKit"
    ;;
  'Linux')
    LDFLAGS="-lX11 -lGL -lXrandr"
    ;;
  '*')
    echo "os not supported ${OS}"
    exit 1
    ;;
esac

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

