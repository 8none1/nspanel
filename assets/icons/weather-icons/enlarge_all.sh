#!/bin/bash
# Inkscape is waaaaaay better at this than imagemagick.

BACKGROUND_COL="#d8d7d2"

for f in *.svg; do
  OUTPUTFILE="${f%.*}.png";
  inkscape -w 200 -y 1 -D $f -e $OUTPUTFILE
  #inkscape -w 200 -e $OUTPUTFILE $f

done
