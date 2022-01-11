#!/bin/bash
# Inkscape is waaaaaay better at this than imagemagick.

#BACKGROUND_COL="#d8d7d2"
BACKGROUND_COL="#ef9a5a"

#mkdir -p 18x18_png
#mkdir -p 22x22_png
mkdir -p 46x46_png
mkdir -p 55x55_png
mkdir -p 138x138_png

for f in *.svg; do
  #OUTPUTFILE="55x55_png/55x55_${f%.*}.png";
  #inkscape -w 55 -d 55 -b "#d8d7d2" -y 1 -e $OUTPUTFILE $f
  #inkscape -w 55 -y 1 -D -b "$BACKGROUND_COL" -y 1 -e $OUTPUTFILE $f

  OUTPUTFILE="46x46_png/46x46_${f%.*}.png";
  inkscape -w 46 -d 46 -b "$BACKGROUND_COL" -D -y 1 -e $OUTPUTFILE $f


  OUTPUTFILE="138x138_png/138x138_${f%.*}.png";
  inkscape -w 138 -d 138 -b "$BACKGROUND_COL" -D -y 1 -e $OUTPUTFILE $f
done

# Special cases
inkscape -w  46 -d 46  -b "$BACKGROUND_COL" -y 1 0-night-clear.svg -e 46x46_png/46x46_0-night-clear.png
inkscape -w 138 -d 138 -b "$BACKGROUND_COL" -y 1 0-night-clear.svg -e 138x138_png/138x138_0-night-clear.png


