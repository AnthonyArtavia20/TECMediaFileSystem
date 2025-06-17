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


    try {
        Raid5Controller controller(4096);

        bool recovered = controller.recoverMissingBlocks();
        if (recovered) {
            std::cerr << "Se reconstruyó al menos un bloque perdido.\n";
        } else {
            // Verificar si el archivo ya había sido guardado anteriormente
            if (!std::filesystem::exists(outputFile)) {
                std::cout << "Archivo no encontrado, procediendo a almacenar en nodos...\n";
                controller.storeFile(inputFile);
            } else {
                std::cout << "No hay bloques perdidos y el archivo ya fue reconstruido previamente.\n";
            }
        }

        controller.rebuildOriginalFile(outputFile);

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
