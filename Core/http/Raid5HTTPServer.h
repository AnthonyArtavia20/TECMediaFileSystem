//Lógica para el servidor, clase encargada de establecer las rutas e iniciar el servidor:

#include "httplib.h" //Esta se descarga desde la págibna oficial, pero la agarré de uhna tarea pasada donde usé esta librería.
#include "controller.h" //Para acceder a los métodos del controlador
#include <nlohmann/json.hpp> //Para poder pasar formatos JSON
#include <iostream> // Para que sepa leer y escribir los datos del usuario.

class Raid5HTTPServer {
public:
  Raid5HTTPServer(Raid5Controller* controller); //Se inicializa con una instancia del controllador
  void start(int port); //Puertos que vendrán desde los xml

private:
  httplib::Server server;
  Raid5Controller* controller; //Instancia de la clase del Controller(metétodos de recuperación, almacenamiento y reconstrucción de PDFS)
  void setup_routes(); //método que settea las oepraciones JSON que se enviaran y recibirán.

};