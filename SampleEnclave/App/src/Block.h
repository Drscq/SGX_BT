#ifndef BLOCK_H
#define BLOCK_H
// This version does not support parallel operations
#include "config.h"
class Block {
public:
    Block();
    ~Block();

    // main methods
    void GenData(const BlockConfig::TYPE_BLOCK_SIZE data_size, const bool is_real, const BlockConfig::TYPE_BLOCK_ID id, std::vector<char>& data);
};
#endif // BLOCK_H