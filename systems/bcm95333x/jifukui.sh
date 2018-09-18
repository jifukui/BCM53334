#!/bin/bash
make clean
make target=loader >loader
cp bcm95333x-loader.bin ./Outputs
make clean
make target=umweb >umweb
cp bcm95333x-umweb.flash ./Outputs
cd Outputs
../../../tools/mkflashimage.pl bcm95333x-loader.bin bcm95333x-umweb.flash bcm95333x_liguo.image
cd ..

