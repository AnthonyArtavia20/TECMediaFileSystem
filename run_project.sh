#!/bin/bash
set -e  # Detiene el script si ocurre un error

#Eliminando la carpeta build para evitar errores del build anterior
echo "Borrando carpeta build"
rm -rf build

# Crear carpeta para binarios de c++
echo "Compilando la vaina a ver si funciona"
cmake -B build -S .
cmake --build build

# Obtener rutas absolutas de los PDFs
PDFS=()
for f in "$@"; do
  if [ ! -f "$f" ]; then
    echo "Error: El archivo $f no existe"
    exit 1
  fi
  PDFS+=("$(realpath "$f")")
done

echo "Iniciando disk node..."
./build/TECMFS-Disk &
sleep 1

echo "Iniciando controller con PDFs: ${PDFS[*]}"
./build/TECMFS-Controller "${PDFS[@]}"

sleep 1

# Espera unos segundos por si los procesos necesitan tiempo
sleep 2

#echo "Ejecutando GUI en Python..."
#cd GUI
#source ../.venv/bin/activate
#exec python3 main.py
