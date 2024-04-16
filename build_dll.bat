gcc -c -o QTImage.o Src/QTImage.c -D QTIMAGE_EXPORTS
gcc -o QTImage.dll -s -shared QTImage.o -Wl,--subsystem,windows