#Envia las solicitudes por medio de https de manera que llame a los archivos c++

import requests

def configuracionBlockSize(size):
  res = requests.post("http://localhost:8080/config", json={"blockSize":size})
  return res.json()

def obtenerEstadoRAID():
    res = requests.get("http://localhost:8080/status")
    return res.json()