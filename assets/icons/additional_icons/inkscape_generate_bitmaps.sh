#!/bin/bash
# Inkscape is waaaaaay better at this than imagemagick.

BACKGROUND_COL="#d8d7d2"

mkdir -p 15x15_png
mkdir -p 20x20_png

#mkdir -p 22x22_png
#mkdir -p 46x46_png
#mkdir -p 55x55_png
#mkdir -p 138x138_png

for f in *.svg; do
  OUTPUTFILE="15x15_png/15x15_${f%.*}.png";
  inkscape -w 15 -d 15 -b "#d8d7d2" -y 1 -e $OUTPUTFILE $f

  OUTPUTFILE="20x20_png/20x20_${f%.*}.png";
  inkscape -w 20 -d 20 -b "#d8d7d2" -y 1 -e $OUTPUTFILE $f
done
