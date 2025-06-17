#include "controller.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./controller archivo1.pdf archivo2.pdf ...\n";
        return 1;
    }

    std::filesystem::create_directory("output");
    Raid5Controller controller(4096);
    

    for (int i = 1; i < argc; ++i) {
        std::string inputFile = argv[i];
        std::string outputFile = "output/" + std::filesystem::path(inputFile).stem().string() + ".pdf";
        std::filesystem::remove(outputFile);  // eliminar pdf viejo
        std::string filename = std::filesystem::path(inputFile).stem().string();

        try {
            std::cout << "\nProcesando archivo: " << inputFile << std::endl;

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

    return 0;
}
