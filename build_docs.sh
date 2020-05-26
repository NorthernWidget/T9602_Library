#! /bin/sh

doxygen doxygen_NW.cfg
moxygen -l cpp -o library_reference.md xml/
rm -rf xml
