#!/bin/bash

# Verifica si se pasa un archivo como argumento
if [ -z "$1" ]; then
  echo "Uso: ./run_project.sh archivo.pdf"
  exit 1
fi

# Crear carpeta para binarios de c++
cmake -S . -B build
cmake --build build

echo "Iniciando disk node..."
(cd build && ./TECMFS-Disk)

echo "Iniciando controller..."
(cd build && ./TECMFS-Controller ../$1) &

echo "Ejecutando GUI en Python..."
cd GUI
source ../.venv/bin/activate
python3 main.py
