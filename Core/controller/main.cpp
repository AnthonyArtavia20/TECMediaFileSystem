#include "controller.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./controller archivo.pdf\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "output/recuperado.pdf";
    std::filesystem::create_directory("output");
    std::filesystem::remove(outputFile);  // eliminar pdf viejo


    try {
        Raid5Controller controller(4096);

        controller.storeFile(inputFile);

        bool recovered = controller.recoverMissingBlocks();
        if (recovered) {
            std::cerr << "Se reconstruyó al menos un bloque perdido.\n";
        } 

        // Reconstruir el archivo PDF desde los nodos
        if (controller.rebuildPdfFromDisks(outputFile)) {
            std::cerr << "Se ha reconstruido el archivo PDF.\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}