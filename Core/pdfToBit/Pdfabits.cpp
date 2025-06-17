#include "PdfaBits.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

// Lee el archivo PDF en un vector de bytes
std::vector<uint8_t> PdfaBit::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
}


// Guardar los datos en binario
std::vector<std::vector<uint8_t>> PdfaBit::partitionIntoBlocks(const std::vector<uint8_t>& data, size_t blockSize) {
    std::vector<std::vector<uint8_t>> blocks;
    size_t totalBlocks = (data.size() + blockSize - 1) / blockSize;
    blocks.reserve(totalBlocks);
    for (size_t i = 0; i < data.size(); i += blockSize) {
        size_t end = std::min(i + blockSize, data.size());
        blocks.emplace_back(data.begin() + i, data.begin() + end);
    }
    return blocks;
}

// Reconstruir el PDF
std::vector<uint8_t> PdfaBit::reconstructFromBlocks(const std::vector<std::vector<uint8_t>>& blocks) {
    std::vector<uint8_t> result;
    for (const auto& block : blocks) {
        result.insert(result.end(), block.begin(), block.end());
    }
    return result;
}


// ---- logica para pasar la informacion al controller ---

// Escribir los bloques que se le pasan al controlador 
void PdfaBit::writeBlock(const std::string& path, int stripeIndex, const std::vector<uint8_t>& data) {
    std::filesystem::create_directories(path);
    std::string filename = path + "/block_" + std::to_string(stripeIndex) + ".bin";
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Leer los bloques que se le pasan al controlador 
std::vector<uint8_t> PdfaBit::readBlock(const std::string& nodePath, int index) {
    std::string filename = nodePath + "/block_" + std::to_string(index) + ".bin";
    return readFile(filename);
}

bool PdfaBit::compare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    return a == b;
}

// ---- reconstruccion de la data perdida por paridad -----
std::vector<uint8_t> PdfaBit::recoverBlockUsingParity(const std::vector<std::vector<uint8_t>>& blocks) {
    if (blocks.empty()) return {};
    std::vector<uint8_t> result = blocks[0];
    for (size_t i = 1; i < blocks.size(); ++i) {
        for (size_t j = 0; j < result.size(); ++j) {
            result[j] ^= blocks[i][j];
        }
    }
    return result;
}

void PdfaBit::rebuildMissingBlock(int stripeIndex, const std::vector<std::string>& nodePaths, int missingNodeIndex) {
    std::vector<std::vector<uint8_t>> presentBlocks;
    for (size_t i = 0; i < nodePaths.size(); ++i) {
        if (i == missingNodeIndex) continue;
        try {
            auto block = PdfaBit::readBlock(nodePaths[i], stripeIndex);
            presentBlocks.push_back(block);
        } catch (const std::exception& ex) {
            std::cerr << "Error leyendo bloque " << stripeIndex << " en nodo " << i << ": " << ex.what() << "\n";
            return;
        }
    }
    auto recovered = PdfaBit::recoverBlockUsingParity(presentBlocks);
    PdfaBit::writeBlock(nodePaths[missingNodeIndex], stripeIndex, recovered);
}

