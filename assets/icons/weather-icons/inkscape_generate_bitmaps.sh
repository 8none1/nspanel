#!/bin/bash
# Inkscape is waaaaaay better at this than imagemagick.

BACKGROUND_COL="#d8d7d2"

#mkdir -p 18x18_png
#mkdir -p 22x22_png
#mkdir -p 46x46_png
mkdir -p 55x55_png
mkdir -p 138x138_png

for f in *.svg; do
  OUTPUTFILE="55x55_png/55x55_${f%.*}.png";
  inkscape -w 55 -d 55 -b "#d8d7d2" -y 1 -e $OUTPUTFILE $f

  OUTPUTFILE="138x138_png/138x138_${f%.*}.png";
  inkscape -w 138 -d 138 -b "#d8d7d2" -y 1 -e $OUTPUTFILE $f
done
