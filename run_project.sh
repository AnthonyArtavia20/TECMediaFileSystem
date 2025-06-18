#!/bin/bash

## Verifica si se pasa un archivo como argumento
#if [ -z "$1" ]; then
#  echo "Uso: ./run_project.sh archivo.pdf"
#  exit 1
#fi

# Crear carpeta para binarios de c++
echo "Compilando la vaina a ver si funciona"
cmake -B build -S .
cmake --build build

echo "Iniciando disk node..."
./build/TECMFS-Disk &

echo "Iniciando controller..."
./build/TECMFS-Controller &

echo "Ejecutando GUI en Python..."
cd GUI
source ../.venv/bin/activate
python3 main.py
