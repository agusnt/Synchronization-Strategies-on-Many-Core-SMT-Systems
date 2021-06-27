#!/bin/bash

cd tinySTM
# Clone TinySTM lib
#git clone https://github.com/patrickmarlier/tinystm.git .
make
cd test
mkdir ht
cd ht
cp ../../../*.cpp .
cp ../../../*.hpp .
cp ../../../Makefile .
make -f Makefile 
mv main ../../../ht
cd ../../../
#rm -rf tmp
