#include <iostream>
#include "../Path.h"
void InitializeElGamalParams() {
    std::cout << "Initializing ElGamal Parameters..." << std::endl;
    
    // Set seed for random number generation
    SetSeed(ElGamalNTLConfig::SEED);
    
    // Generate prime P
    GenPrime(ElGamalNTLConfig::P, ElGamalNTLConfig::KEY_SIZE);
    ZZ_p::init(ElGamalNTLConfig::P);
//     std::cout << "P: " << ElGamalNTLConfig::P << std::endl;
    
    // Convert G to G_p
    ElGamalNTLConfig::G_p = conv<ZZ_p>(ElGamalNTLConfig::G);
//     std::cout << "G_p: " << ElGamalNTLConfig::G_p << std::endl;
    
    // Generate private key X and convert to X_p
    ElGamalNTLConfig::X = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::X_p = conv<ZZ_p>(ElGamalNTLConfig::X);
//     std::cout << "X_p: " << ElGamalNTLConfig::X_p << std::endl;
    
    // Calculate public key Y and convert to Y_p
    ElGamalNTLConfig::Y = PowerMod(ElGamalNTLConfig::G, ElGamalNTLConfig::X, ElGamalNTLConfig::P);
    ElGamalNTLConfig::Y_p = conv<ZZ_p>(ElGamalNTLConfig::Y);
//     std::cout << "Y_p: " << ElGamalNTLConfig::Y_p << std::endl;
    
    // Generate random K and convert to K_p
    ElGamalNTLConfig::K = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::K_p = conv<ZZ_p>(ElGamalNTLConfig::K);
//     std::cout << "K_p: " << ElGamalNTLConfig::K_p << std::endl;
     ElGamalNTLConfig::GPowK = power(ElGamalNTLConfig::G_p, ElGamalNTLConfig::K);
     ElGamalNTLConfig::YPowK = power(ElGamalNTLConfig::Y_p, ElGamalNTLConfig::K);
}

void decrypt_print_bucket(std::string bucket_path) {
    std::ifstream bucket_file(bucket_path, std::ios::binary);
    if (!bucket_file) {
        std::cerr << "Error opening file: " << bucket_path << std::endl;
        return;
    }
    std::vector<char> buffer(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    bucket_file.read(buffer.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    bucket_file.close();
    std::vector<std::vector<BIGNUM*>> bucketCiphertexts;
    bucketCiphertexts.resize(2);
    bucketCiphertexts[0].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    bucketCiphertexts[1].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    for (int i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; ++i) {
        bucketCiphertexts[0][i] = BN_new();
        bucketCiphertexts[1][i] = BN_new();
    }
    ElGamal_parallel_ntl elgamal(ServerConfig::num_threads, BlockConfig::BLOCK_SIZE);
    elgamal.ConvertVecCharCipher2VecBN(buffer, bucketCiphertexts);
    std::vector<BIGNUM*> decrypted_data;
    decrypted_data.resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    for (int i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; ++i) {
        decrypted_data[i] = BN_new();
    }
    elgamal.ParallelDecrypt(bucketCiphertexts[0], bucketCiphertexts[1], decrypted_data);
    for (int i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; ++i) {
        std::cout << "Decrypted data " << i << ": " << BN_bn2dec(decrypted_data[i]) << std::endl;
    }
    for (int i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; ++i) {
        BN_free(decrypted_data[i]);
    }
    for (int i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; ++i) {
        BN_free(bucketCiphertexts[0][i]);
        BN_free(bucketCiphertexts[1][i]);
    }
    std::cout << "Decrypted bucket data printed." << std::endl;
}

int main() {
    // Initialize ElGamal parameters
    InitializeElGamalParams();
    // Path path(ClientConfig::PATH_ID, PathConfig::HEIGHT);
    // path.GenEvictPath(ClientConfig::PATH_ID, PathConfig::HEIGHT);
    // // check the bIDs in the path
    // std::cout << "The bIDs in the path: ";
    // for (auto & bID : path.bIDs) {
    //     std::cout << bID << " ";
    // }
    // std::cout << std::endl;
    // for (auto & bID : path.bIDs) {
    //     std::string bucket_path = "../../../" + BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(bID);
    //     std::cout << "Bucket path: " << bucket_path << std::endl;
    //     decrypt_print_bucket(bucket_path);
    // }
    int num_buckets = (1 << PathConfig::HEIGHT) - 1;
    for (int i = 0; i < num_buckets; ++i) {
        std::string bucket_path = "../../../" + BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(i);
        std::cout << "Bucket path: " << bucket_path << std::endl;
        decrypt_print_bucket(bucket_path);
    }
    
    
}