#include <string>
#include <vector>
#include <cstdint>

class PdfaBit {
public:
    static std::vector<uint8_t> readFile(const std::string& filename);
    static void writeFile(const std::string& filename, const std::vector<uint8_t>& data);
    static std::vector<std::vector<uint8_t>> partitionIntoBlocks(const std::vector<uint8_t>& data, size_t blockSize);
    static std::vector<uint8_t> reconstructFromBlocks(const std::vector<std::vector<uint8_t>>& blocks);
    static void writeBlock(const std::string& nodePath, int index, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> readBlock(const std::string& nodePath, int index);
    static bool compare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);

    static std::vector<uint8_t> recoverBlockUsingParity(const std::vector<std::vector<uint8_t>>& availableBlocks);
    static void rebuildMissingBlock(int stripeIndex, const std::vector<std::string>& nodePaths, int missingNodeIndex);
};