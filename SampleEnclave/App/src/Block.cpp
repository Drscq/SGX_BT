#include "Block.h"

Block::Block() {}

Block::~Block() {}

void Block::GenData(const BlockConfig::TYPE_BLOCK_SIZE data_size, const bool is_real, const BlockConfig::TYPE_BLOCK_ID id, std::vector<char>& data) {
    if (!is_real) {
        // Generate dummy data with one data
        data = ElGamalConfig::generate_one_data(data_size);
    } else {
        // Generate random data
        data = ElGamalConfig::generate_binary_data(data_size);
        // memcpy the id to the data
        std::memcpy(data.data(), &id, sizeof(id));
        // std::cout << "Block ID: " << id << std::endl;
    }
}