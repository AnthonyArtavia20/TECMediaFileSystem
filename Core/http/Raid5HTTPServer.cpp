//Definición de las operaciones del JSON

#include "Raid5HTTPServer.h"
#include <fstream>


using json = nlohmann::json; //alias para simplifar la sintaxys

Raid5HTTPServer::Raid5HTTPServer(Raid5Controller* controller) : controller(controller) {}

void Raid5HTTPServer::setup_routes() {
  //Definición de las oraociones del servidor y que harán:

  server.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
    json j;
    j["status"] = "RAID OK"; //este se puede modificar para distintos tipos de información
    res.set_content(j.dump(), "application/json");
  });

  server.Get("/files", [this](const httplib::Request&, httplib::Response& res) {
      // Simulación inicial
      json j;
      j["files"] = { "test1.pdf", "test2.pdf" };
      res.set_content(j.dump(), "application/json");
  });

  server.Get("/download", [this](const httplib::Request& req, httplib::Response& res) {
      auto filename = req.get_param_value("name");
      std::string output = "/tmp/" + filename;
      if (controller->rebuildPdfFromDisks(filename, output)) {
          std::ifstream in(output, std::ios::binary);
          std::ostringstream sstr;
          sstr << in.rdbuf();
          res.set_content(sstr.str(), "application/pdf");
      } else {
          res.status = 404;
          res.set_content("Archivo no encontrado", "text/plain");
      }
  });

  server.Post("/upload", [this](const httplib::Request& req, httplib::Response& res) {
      auto filename = req.get_param_value("name");
      std::string temp = "/tmp/" + filename;
      std::ofstream out(temp, std::ios::binary);
      out.write(req.body.c_str(), req.body.size());
      out.close();
      controller->storeFile(temp);
      res.set_content("Archivo subido y almacenado en RAID", "text/plain");
  });
}

void Raid5HTTPServer::start(int port) {
  setup_routes();
  std::cout << "Servidor HTTP escuchando en puerto " << port << std::endl;
  server.listen("0.0.0.0", port);
}
