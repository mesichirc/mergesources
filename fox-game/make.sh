#/bin/sh

LFLAGS="-lX11 -lXrandr"
BUNDLE="./app/"
EXE="foxy"

gcc -o "$BUNDLE/$EXE" $LFLAGS main.c
