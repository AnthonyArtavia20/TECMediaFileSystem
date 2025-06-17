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
    std::filesystem::remove(outputFile);  //eliminar archivo ya creado

    try {
        Raid5Controller controller(4096);

        controller.storeFile(argv[1]);

        
        if (controller.recoverMissingBlocks()) {
            std::cerr << "Se reconstruyo un bloque.\n";
        }
        
        if (!controller.rebuildPdfFromDisks(outputFile)) {
            std::cerr << "Error al reconstruir el archivo PDF.\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
