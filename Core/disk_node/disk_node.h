// header del servidor local
#ifndef DISK_NODE_H
#define DISK_NODE_H

#include <string>

class Disk_Node {
public:
    Disk_Node(const std::string& configPath);
    void initializeStorage();
    void printInfo();

private:
    std::string ip;
    int port;
    std::string storagePath;
    int blockSize;
    int diskSize;
    int numBlocks;

    void loadConfig(const std::string& configPath);
};

#endif
