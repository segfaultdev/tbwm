#!/usr/bin/sh

gcc $(find . -name "*.c") -Iinclude -o tbwm -lX11 -DI_X11 -s -Ofast
