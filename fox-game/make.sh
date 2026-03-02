#/bin/sh

LFLAGS="-lX11 -lXrandr -lm"
BUNDLE="./app"
EXE="foxy"
FONTS="fonts"
DEBUG_FONT="debug_font.h"

prepare_fonts()
{
  if [ ! -f "$BUNDLE/$FONTS" ] || [ $(find -name fox-font.c -newer "$BUNDLE/$FONTS") ]; then
    gcc -o "$BUNDLE/$FONTS" fox-font.c
    rm $DEBUG_FONT
  fi
  if [ ! -f $DEBUG_FONT ]; then
    "./${BUNDLE}/${FONTS}" $DEBUG_FONT
  fi
}

case $1 in
  'clear')
    rm $DEBUG_FONT
    rm "$BUNDLE/$FONTS"
    rm "$BUNDLE/$EXE"
    ;;
  'build')
    prepare_fonts
    gcc -g -o "$BUNDLE/$EXE" $LFLAGS main.c
    ;;
  'prepare-fonts')
    prepare_fonts
    ;;
esac
