#include "controller.h"
#include "../../Core/pdfToBit/Pdfabits.cpp"

#include <iostream>

#include <pugixml.hpp>
#include <filesystem>
#include <algorithm>

Raid5Controller::Raid5Controller(size_t blockSize) : blockSize(blockSize) {
    std::string baseConfigPath = std::filesystem::current_path().string() + "/Core/config/";

    std::vector<std::string> configFiles = {
        baseConfigPath + "disk1.xml",
        baseConfigPath + "disk2.xml",
        baseConfigPath + "disk3.xml",
        baseConfigPath + "disk4.xml"
    };

    for (int i = 0; i < configFiles.size(); ++i) {
        pugi::xml_document doc;
        if (!doc.load_file(configFiles[i].c_str())) {
            throw std::runtime_error("No se pudo cargar el archivo: " + configFiles[i]);
        }

        std::string rawPath = doc.child("DiskNode").child("StoragePath").text().as_string();

        // Esto fuerza que la ruta sea desde el root de tu proyecto correctamente
        std::filesystem::path projectRoot = std::filesystem::current_path();  // Ya estás parado aquí al correr ./build/TECMFS-Controller
        std::filesystem::path diskPath = projectRoot / rawPath;

        nodes.push_back(diskPath.string());
        diskStates[i + 1] = true;
    }

    std::cout << "[DEBUG] Discos cargados: " << nodes.size() << std::endl;
    for (int i = 0; i < nodes.size(); ++i) {
        std::cout << " - Disco " << (i + 1) << " => " << nodes[i] << std::endl;
    }
}

void Raid5Controller::storeFile(const std::string& filepath) {
    auto data = PdfaBit::readFile(filepath);
    auto blocks = PdfaBit::partitionIntoBlocks(data, blockSize);

    std::string baseFilename = std::filesystem::path(filepath).stem().string();

    int totalDisks = nodes.size();
    int dataDisks = totalDisks - 1;
    int stripeCount = (blocks.size() + dataDisks - 1) / dataDisks;
    int blockIndex = 0;

    for (int stripe = 0; stripe < stripeCount; ++stripe) {
        int parityDisk = stripe % totalDisks;
        std::vector<std::vector<uint8_t>> stripeData(totalDisks); // vacíos por defecto

        // Rellenar los bloques de datos (omitimos disco de paridad)
        for (int i = 0; i < totalDisks; ++i) {
            if (i == parityDisk || blockIndex >= blocks.size()) continue;
            stripeData[i] = blocks[blockIndex++];
        }

        // Calcular paridad XOR de los bloques de datos
        std::vector<uint8_t> parity(blockSize, 0);
        for (int i = 0; i < totalDisks; ++i) {
            if (i == parityDisk || stripeData[i].empty()) continue;
            for (size_t j = 0; j < blockSize; ++j) {
                parity[j] ^= stripeData[i][j];
            }
        }
        stripeData[parityDisk] = parity;

        // Escribir bloques reales (evitamos escribir vacíos)
        for (int i = 0; i < totalDisks; ++i) {
            if (!stripeData[i].empty()) {
                PdfaBit::writeBlock(nodes[i], baseFilename, stripe, stripeData[i]);
            }
        }
    }

    std::cout << "Archivo almacenado usando RAID 5." << std::endl;
}

bool Raid5Controller::recoverMissingBlocks(const std::string& filename) {
    bool reconstructedBlock = false;
    int totalDisks = nodes.size();
    int maxStripes = 17;

    for (int stripe = 0; stripe < maxStripes; ++stripe) {
        int missingIndex = -1;
        std::vector<std::vector<uint8_t>> presentBlocks(totalDisks);

        for (int i = 0; i < totalDisks; ++i) {
            try {
                presentBlocks[i] = PdfaBit::readBlock(nodes[i], filename, stripe);
                if (presentBlocks[i].empty()) missingIndex = i;
            } catch (...) {
                missingIndex = i;
            }
        }

        int missingCount = std::count_if(presentBlocks.begin(), presentBlocks.end(),
                                         [](const auto& b) { return b.empty(); });

        if (missingCount == 1 && missingIndex != -1) {
            std::cout << "Recuperando bloque faltante en stripe " << stripe << " del nodo " << missingIndex << "\n";

            std::vector<std::vector<uint8_t>> available;
            for (int i = 0; i < totalDisks; ++i) {
                if (i != missingIndex) available.push_back(presentBlocks[i]);
            }

            auto recovered = PdfaBit::recoverBlockUsingParity(available);
            PdfaBit::writeBlock(nodes[missingIndex], filename, stripe, recovered);
            reconstructedBlock = true;
        }
    }

    return reconstructedBlock;
}


bool Raid5Controller::rebuildPdfFromDisks(const std::string& filename, const std::string& outputFilename) {
    int totalDisks = nodes.size();
    int maxStripes = 17;
    std::vector<std::vector<uint8_t>> fullData;

    for (int stripe = 0; stripe < maxStripes; ++stripe) {
        int missingIndex = -1;
        std::vector<std::vector<uint8_t>> presentBlocks(totalDisks);

        for (int i = 0; i < totalDisks; ++i) {
            try {
                presentBlocks[i] = PdfaBit::readBlock(nodes[i], filename, stripe);
                if (presentBlocks[i].empty()) missingIndex = i;
            } catch (...) {
                missingIndex = i;
            }
        }

        int missingCount = std::count_if(presentBlocks.begin(), presentBlocks.end(),
                                         [](const auto& b) { return b.empty(); });

        if (missingCount > 1) {
            std::cerr << "No se puede reconstruir el stripe " << stripe << ": faltan múltiples bloques.\n";
            return false;
        }

        if (missingCount == 1 && missingIndex != -1) {
            std::vector<std::vector<uint8_t>> available;
            for (int i = 0; i < totalDisks; ++i) {
                if (i != missingIndex) available.push_back(presentBlocks[i]);
            }
            auto recovered = PdfaBit::recoverBlockUsingParity(available);
            presentBlocks[missingIndex] = recovered;
            std::cerr << "Bloque reconstruido en stripe " << stripe
                      << " del nodo " << missingIndex << " desde paridad.\n";
        }

        int parityIndex = stripe % totalDisks;
        for (int i = 0; i < totalDisks; ++i) {
            if (i != parityIndex) {
                fullData.push_back(presentBlocks[i]);
            }
        }
    }

    auto reconstructedPdf = PdfaBit::reconstructFromBlocks(fullData);

    std::ofstream out(outputFilename, std::ios::binary);
    if (!out) {
        std::cerr << "Error al crear el archivo PDF de salida.\n";
        return false;
    }

    out.write(reinterpret_cast<const char*>(reconstructedPdf.data()), reconstructedPdf.size());
    out.close();

    std::cout << "PDF reconstruido con éxito: " << outputFilename << "\n";
    return true;
}

bool Raid5Controller::deleteFile(const std::string& filename) {
    bool success = true;
    std::string prefix = "block_" + filename + "_";

    for (const auto& path : nodes) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string fname = entry.path().filename().string();
                    if (fname.find(prefix) == 0) {
                        std::filesystem::remove(entry.path());
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Al eliminar bloques de " << path << ": " << e.what() << "\n";
            success = false;
        }
    }

    // También borra el PDF reconstruido (opcional)
    std::filesystem::path output = "Core/storage/output/" + filename + ".pdf";
    if (std::filesystem::exists(output)) {
        std::filesystem::remove(output);
    }

    return success;
}


void Raid5Controller::setBlockSize(size_t newBlockSize) {
    blockSize = newBlockSize;
    std::cout << "Nuevo blockSize asignado: " << newBlockSize << std::endl;
}

//Métodos para saber el estado de los RAIDS;
void Raid5Controller::startDisk(int id) {
    if (id > 0 && id <= (int)diskStates.size()) {
        std::cout << "[DEBUG] startDisk(" << id << ")\n";
        std::cout << "[DEBUG] Estado actual de disco " << id << ": " << std::boolalpha << diskStates[id - 1] << std::endl;
        diskStates[id - 1] = true;
    }
}

void Raid5Controller::stopDisk(int id) {
    if (id > 0 && id <= (int)diskStates.size()) {
        std::cout << "[DEBUG] stopDisk(" << id << ")\n";
        std::cout << "[DEBUG] Estado actual de disco " << id << ": " << std::boolalpha << diskStates[id - 1] << std::endl;
        diskStates[id - 1] = false;
    }
}

void Raid5Controller::setDiskState(int id, bool active) {
    if (diskStates.count(id)) {
        diskStates[id] = active;
        std::cout << "Disco " << id << (active ? " activado" : " desactivado") << "." << std::endl;
    } else {
        std::cerr << "[ERROR] ID de disco inválido: " << id << std::endl;
    }
}

bool Raid5Controller::getDiskState(int id) const {
    auto it = diskStates.find(id);
    if (it != diskStates.end()) {
        std::cout << "[DEBUG] getDiskState(" << id << ") = " << std::boolalpha << it->second << std::endl;
        return it->second;
    } else {
        std::cerr << "[DEBUG] getDiskState(" << id << ") = NOT FOUND" << std::endl;
        return false;
    }
}

int Raid5Controller::getActiveDiskCount() const {
    int count = 0;
    for (const auto& [id, state] : diskStates) {
        if (state) ++count;
    }
    return count;
}

int Raid5Controller::getDiskCount() const {
    return nodes.size();
}