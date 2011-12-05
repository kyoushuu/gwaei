#!/bin/sh

libtoolize --force --install --copy \
&& autoreconf --force --install
