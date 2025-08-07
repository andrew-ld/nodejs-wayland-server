#!/bin/bash

set +xe

meson install -C ./builddir

find ./builddir/install/ -type f -name "*.so" -exec cp -p {} ./artifacts/ \;
find ./builddir/install/ -type f -name "*.node" -exec cp -p {} ./artifacts/ \;
