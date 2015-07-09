#!/bin/bash

# Writen by Caio Oliveira aka Caio99BR <caiooliveirafarias0@gmail.com>

# How to Use:
# . patch link.to.commit

# This script can apply three commits at once.
# . patch link.to.first.commit link.to.second.commit link.to.third.commit

# First patch
curl $1.patch | git am

# Second Patch
if ! [ "$2" == "" ]; then
curl $2.patch | git am
fi

# Third Patch
if ! [ "$3" == "" ]; then
curl $3.patch | git am
fi
