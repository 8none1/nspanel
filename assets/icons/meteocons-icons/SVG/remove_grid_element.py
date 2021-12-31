#!/usr/bin/env python3

from xml.dom import minidom
import sys

file_name = sys.argv[1]

dom = minidom.parse(file_name)

# First remove the grid.  We expect it to be nested thusly:
# root -> id="Grid_1_" -> id="Layer_1_1_" -> id="Grid"
# or thereabouts.

for el in dom.getElementsByTagName('g'):
  if el.getAttribute('id') == "Grid":
    p = el.parentNode.parentNode
    if 'grid' in p.getAttribute('id').lower():
      p.parentNode.removeChild(p)

# Next we change the fill colour to black

for el in dom.getElementsByTagName('path'):
  if el.getAttribute('fill') == '#1D1D1B' :
    el.setAttribute('fill', '#000000')

#print(dom.toxml())

file_out = "processed/"+file_name
file_handle = open(file_out, "w")
dom.writexml(file_handle)
file_handle.close()

