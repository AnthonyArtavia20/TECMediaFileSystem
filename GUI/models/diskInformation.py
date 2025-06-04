# Formato a mostrar en cada disco, técnicamente su estado:
#Ocupado, libre, desconectado conectado, uso(porcentaje de uso)

class DiskInformationClass:

  def __init__(self, IdDisco, nombreDisco, estadoDisco, bloquesUsados, bloquesTotales,esParidad=False):
    self.IdDisco = IdDisco
    self.nombre = nombreDisco
    self.estado = estadoDisco
    self.bloquesEnUso = bloquesUsados
    self.totalidadBloques = bloquesTotales
    self.paridad = esParidad
    self.porcentajeUso = calcularPorcentajeUso

    def calcularPorcentajeUso(self):
      if bloquesTotales == 0:
        return 0
      else:
        return (self.bloquesUsados / self.bloquesTotales)*100