#include "controller.h"
#include "../http/Raid5HTTPServer.h"
#include <iostream>
#include <filesystem>
#include <thread>
using namespace std;

int main(int argc, char* argv[]) {

    if (argc == 1) {
    std::cout << "Esperando conexiones desde la GUI en http://localhost:8080\n";
    std::cout << "Ya no se admiten archivos por línea de comandos.\n";
    }

    string basePath = filesystem::current_path().string(); //para obtener la ruta actual donde se ejcuta el programa
    basePath = basePath + "/../Core/storage/";
    std::filesystem::create_directories(basePath + "output/");

    Raid5Controller controller(4096);

    //Inicializacióan y conexión del servidor HTTP embebido
    cout << "Servidor HTTP embebido corriendo en http://localhost:8080\n";
    std::thread http_thread([&controller]() {
    std::cout << "[DEBUG] Iniciando hilo del servidor HTTP...\n";
    try {
        Raid5HTTPServer server(&controller);
        server.start(8080);
        std::cout << "[DEBUG] Servidor HTTP finalizó correctamente (esto no debería pasar normalmente).\n";
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Excepción en el servidor HTTP: " << ex.what() << "\n";
    } catch (...) {
        std::cerr << "[ERROR] Excepción desconocida en el servidor HTTP.\n";
    }
});

    for (int i = 1; i < argc; ++i) {
        std::string inputFile = argv[i];
        std::string outputFile = basePath + "output/" + std::filesystem::path(inputFile).stem().string() + ".pdf";
        std::filesystem::remove(outputFile);  // eliminar pdf viejo
        std::string filename = std::filesystem::path(inputFile).stem().string();

        try {
            std::cout << "\nProcesando archivo: " << inputFile << std::endl;
            std::cout << "[DEBUG] Iniciando hilo del servidor HTTP...\n";

            bool recovered = controller.recoverMissingBlocks(filename);
            if (recovered) {
                std::cerr << "Se reconstruyó al menos un bloque perdido.\n";
            } else {
                controller.storeFile(inputFile);
            }

            if (controller.rebuildPdfFromDisks(filename, outputFile)) {
                std::cerr << "Se ha reconstruido el archivo: " << outputFile << "\n";
            }

        } catch (const std::exception& ex) {
            std::cerr << "Error al procesar " << inputFile << ": " << ex.what() << std::endl;
        }
    }

    http_thread.join(); //Para que no haga otra cosa hasta que termine
    return 0;
}
