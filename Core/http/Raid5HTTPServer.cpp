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
    // Estado general del sistema
    server.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
        json j;
        int active_disks = controller->getActiveDiskCount();
        j["active_disks"] = active_disks;
        j["status"] = (active_disks >= 3) ? "OK" : (active_disks == 2 ? "DEGRADED" : "FAILED");

        json disk_states_json = json::array();
        int disk_count = controller->getDiskCount(); // Este método debe retornar nodes.size()
        for (int i = 1; i <= disk_count; ++i) {
            bool state = controller->getDiskState(i);
            std::cout << "[DEBUG] getDiskState(" << i << ") = " << std::boolalpha << state << "\n";
            disk_states_json.push_back({
                {"id", i},
                {"active", state}
            });
        }
        j["disks"] = disk_states_json;
        res.set_content(j.dump(), "application/json");
    });

    // Listar archivos simulados
    server.Get("/files", [this](const httplib::Request&, httplib::Response& res) {
        json j;
        j["files"] = { "test1.pdf", "test2.pdf" };
        res.set_content(j.dump(), "application/json");
    });

    // Descargar PDF reconstruido
    server.Get("/download", [this](const httplib::Request& req, httplib::Response& res) {
        auto filename = req.get_param_value("name");
        std::string clean_filename = std::filesystem::path(filename).stem().string();
        std::string output = "/tmp/" + filename;

        if (controller->rebuildPdfFromDisks(clean_filename, output)) {
            std::ifstream in(output, std::ios::binary);
            std::ostringstream sstr;
            sstr << in.rdbuf();
            res.set_content(sstr.str(), "application/pdf");
        } else {
            res.status = 404;
            res.set_content("Archivo no encontrado", "text/plain");
        }
    });

    // Subir PDF - versión corregida para multipart/form-data
    server.Post("/upload", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_file("file")) {
            const auto& file = req.get_file_value("file");
            std::string filename = std::filesystem::path(file.filename).stem().string();
            std::string temp_path = "/tmp/" + filename;

            std::ofstream ofs(temp_path, std::ios::binary);
            ofs << file.content;
            ofs.close();

            controller->storeFile(temp_path);

            // Reconstruir en /tmp para tener el PDF entero accesible
            std::string reconstructed = "/tmp/" + filename;
            bool ok = controller->rebuildPdfFromDisks(filename, reconstructed);

            if (ok) {
                std::cout << "[INFO] Archivo reconstruido en: " << reconstructed << std::endl;
            } else {
                std::cerr << "[ERROR] No se pudo reconstruir el archivo tras subida." << std::endl;
            }

            res.set_content("Archivo subido y almacenado en RAID", "text/plain");
        } else {
            res.status = 400;
            res.set_content("No se encontró archivo en la solicitud.", "text/plain");
        }
    });

    // Configurar blockSize
    server.Post("/config", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = nlohmann::json::parse(req.body);
        size_t newBlockSize = body["blockSize"];
        controller->setBlockSize(newBlockSize);
        res.set_content("{\"status\": \"ok\"}", "application/json");
    });

    server.Post("/stop_disk", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("id")) {
            int disk_id = std::stoi(req.get_param_value("id"));
            controller->stopDisk(disk_id);
            res.set_content("OK", "text/plain");
        } else {
            res.status = 400;
            res.set_content("Missing disk id", "text/plain");
        }
    });

    server.Post("/start_disk", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("id")) {
            int disk_id = std::stoi(req.get_param_value("id"));
            controller->startDisk(disk_id);
            res.set_content("OK", "text/plain");
        } else {
            res.status = 400;
            res.set_content("Missing disk id", "text/plain");
        }
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
