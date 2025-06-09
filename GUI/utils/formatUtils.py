#Podría utilizarse para poder hacer los formateos de los bytes o mostrar una salida bonita
#Confirmar error o mostrar mensajes de sugerencia para poder indicar algún estado o similar

def format_bytes(size):
  # 2**10 = 1024

  for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
    if size < 1024.0:
      return f"{size: .2f} {unit}"
    size /= 1024.0