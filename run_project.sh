#!/bin/bash

cmake -S . -B build
cmake --build build

echo "Iniciando disk node..."
(cd build && ./TECMFS-Disk)

echo "Iniciando controller..."
(cd build && ./TECMFS-Controller ../PDFprueba.pdf) &

echo "Ejecutando GUI en Python..."
cd GUI
source ../.venv/bin/activate
python3 main.py
