#!/bin/bash

cp main.cpp stm
cd stm
./build_stm.sh
mv bst ../bst.stm
rm main.cpp
cd ..
