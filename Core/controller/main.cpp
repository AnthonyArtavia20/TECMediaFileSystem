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
        std::cout << "Archivo almacenado usando RAID 5.\n";

        controller.recoverMissingBlocks();

        if (controller.rebuildPdfFromDisks(outputFile)) {
            std::cout << "PDF reconstruido correctamente.\n";
        } else {
            std::cerr << "Error al reconstruir el archivo PDF.\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
