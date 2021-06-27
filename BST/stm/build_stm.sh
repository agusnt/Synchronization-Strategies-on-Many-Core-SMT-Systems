#!/bin/bash

# Create directory
mkdir tinySTM
cd tinySTM
# Clone TinySTM lib
git clone https://github.com/patrickmarlier/tinystm.git .
# Build TinySTM
make
# Copy BST source files
cd test
mkdir bst
cd bst
cp ../../../*.cpp .
cp ../../../*.hpp .
cp ../../../Makefile .
# Build BST
make -f Makefile 
# Copy BST data
mv main ../../../bst
cd ../../../
rm -rf tmp
