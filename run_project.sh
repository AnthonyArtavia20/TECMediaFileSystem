#!/bin/bash

echo "Compilando backend..."
mkdir -p build
cd build
cmake ..
make

echo "Iniciando disk node..."
./TECMFS-Disk &

echo "Iniciando controller..."
./TECMFS-Controller &

cd ..

echo "Ejecutando GUI en Python..."
cd GUI
source ../.venv/bin/activate
python3 main.py
