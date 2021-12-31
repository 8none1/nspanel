#!/bin/bash
# This will use imagemagick to resize and recolour the SVGs to the required size for the NSPanel UI
# It expects the images to have the lines already converted to pure black

BACKGROUND_COL="#d8d7d2"
mkdir -p 22x22_png
mkdir -p 55x55_png
mkdir -p 138x138_png

for f in *.svg; do
  OUTPUTFILE="22x22_png/22x22_${f%.*}.png";
  convert $f -background "$BACKGROUND_COL" -fill "$BACKGROUND_COL" -flatten +opaque black -alpha remove -resize 22x22 -strip $OUTPUTFILE
  OUTPUTFILE="55x55_png/55x55_${f%.*}.png";
  convert $f -background "$BACKGROUND_COL" -fill "$BACKGROUND_COL" -flatten +opaque black -alpha remove -resize 55x55 -strip $OUTPUTFILE
  OUTPUTFILE="138x138_png/138x138_${f%.*}.png";
  convert $f -background "$BACKGROUND_COL" -fill "$BACKGROUND_COL" -flatten +opaque black -alpha remove -resize 138x138 -strip $OUTPUTFILE
done
