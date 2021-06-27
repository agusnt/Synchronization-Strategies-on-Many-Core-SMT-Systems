#!/bin/bash

cp main.cpp stm
cd stm
./build_stm.sh
mv ht ../ht.stm
rm main.cpp
cd ..
