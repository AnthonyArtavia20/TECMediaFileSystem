//Definición de las operaciones del JSON

#include "Raid5HTTPServer.h"
#include "controller.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>


using json = nlohmann::json; //alias para simplifar la sintaxys

Raid5HTTPServer::Raid5HTTPServer(Raid5Controller* controller) : controller(controller) {}

void Raid5HTTPServer::setup_routes() {
  //Definición de las oraociones del servidor y que harán:

  //Estado general del sistema
  server.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
    json j;

    int active_disks = controller->getActiveDiskCount();
    j["active_disks"] = active_disks;
    j["status"] = (active_disks >= 3) ? "OK" : (active_disks == 2 ? "DEGRADED" : "FAILED");

    json disk_states_json = json::array();
    for (int i = 0; i < 5; ++i) {
        disk_states_json.push_back({
            {"id", i},
            {"active", controller->getDiskState(i)}
        });
    }

    j["disks"] = disk_states_json;

    res.set_content(j.dump(), "application/json");
});

  //Encargado de listar los archivos
  server.Get("/files", [this](const httplib::Request&, httplib::Response& res) {
      // Simulación inicial
      json j;
      j["files"] = { "test1.pdf", "test2.pdf" };
      res.set_content(j.dump(), "application/json");
    });

  //Descargar PDF reconstruido desde los discos
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

  //Para subir archivos
  server.Post("/upload", [this](const httplib::Request& req, httplib::Response& res) {
      auto filename = req.get_param_value("name");
      std::string temp = "/tmp/" + filename;
      std::ofstream out(temp, std::ios::binary);
      out.write(req.body.c_str(), req.body.size());
      out.close();
      controller->storeFile(temp);
      res.set_content("Archivo subido y almacenado en RAID", "text/plain");
  });

  //El siguiente endpoint sirve para poder settear un único blocksize enviado desde la interfaz GUI en lugar de usar por defecto el antiguo 4096 a mano
  //como todos los discos tienen el mismo tamaño se envia solo este.
  server.Post("/config", [this](const httplib::Request& req, httplib::Response& res) {
    auto body = nlohmann::json::parse(req.body);
    size_t newBlockSize = body["blockSize"];
    controller->setBlockSize(newBlockSize);
    res.set_content("{\"status\": \"ok\"}", "application/json");
  });

  // Encender disco específico
  server.Post("/start_disk", [this](const httplib::Request& req, httplib::Response& res) {
      int id = std::stoi(req.get_param_value("id"));
      controller->setDiskState(id, true);
      res.set_content("{\"status\": \"disk started\"}", "application/json");
  });

  // Apagar disco específico
  server.Post("/stop_disk", [this](const httplib::Request& req, httplib::Response& res) {
      int id = std::stoi(req.get_param_value("id"));
      controller->setDiskState(id, false);
      res.set_content("{\"status\": \"disk stopped\"}", "application/json");
  });

  // Obtener cantidad de discos activos
  server.Get("/disk_count", [this](const httplib::Request&, httplib::Response& res) {
      int count = controller->getActiveDiskCount();
      json j;
      j["active_disks"] = count;
      res.set_content(j.dump(), "application/json");
  });
}

void Raid5HTTPServer::start(int port) {
    setup_routes();
    std::cout << "Servidor HTTP escuchando en puerto " << port << std::endl;
    server.listen("0.0.0.0", port);
}