#!/bin/sh

cp schemes/c/$1
./Coco Coco.atg
unix2dos $1
expand $1 > $1.out0
expand schemes/c/$1 > $1.out1
diff -b -u $1.out0 $1.out1
rm $1.out0 $1.out1
