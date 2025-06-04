# El formato que va a tener cada archivo a mostrar en la interfáaz: Nombre, tamaño, fecha, etc

class File_Entry:
  def __init__(self, nombreDisco, tamaño, fechaActual):
    self.nombre = nombreDisco
    self.tamaño = tamaño
    self.fecha = fechaActual

