gcc -c -o QTImage.o Src/QTImage.c -D QTIMAGE_EXPORTS
gcc -c -o BMPImage.o Src/BMPImage.c
gcc -c -o QuadTreeNode.o Src/QuadTreeNode.c
gcc -c -o colorTypes.o Src/colorTypes.c

gcc -o QTImage.dll QTImage.o BMPImage.o QuadTreeNode.o colorTypes.o  -s -shared 