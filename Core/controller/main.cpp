#include "controller.h"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./controller archivo.pdf\n";
        return 1;
    }

    // el controller recibe como argumento el nombre del archivo a leer
    try {
        Raid5Controller controller(4096);
        controller.storeFile(argv[1]);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}


