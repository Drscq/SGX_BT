#include "ElGamal_parallel_ntl.h"
#include "config.h"
#include <iostream>
#include <cassert>
#include <NTL/BasicThreadPool.h>
#include <sched.h>    // For sched_getcpu()
// ElGamal_parallel_ntl::ElGamal_parallel_ntl() {
//     // std::cout << "ElGamal_parallel_ntl constructor" << std::endl;
// }

ElGamal_parallel_ntl::ElGamal_parallel_ntl(size_t num_threads, size_t data_size) :
    num_threads(num_threads), data_size(data_size) {
    this->chunk_size = ElGamalNTLConfig::CHUNK_SIZE;
    this->per_ciphertext_size = ElGamalNTLConfig::PER_CIPHERTEXT_SIZE;
    this->p = ElGamalNTLConfig::P;
    this->g = ElGamalNTLConfig::G;
    this->g_p = ElGamalNTLConfig::G_p;
    // std::cout << "g_p: " << this->g_p << std::endl;
    this->x = ElGamalNTLConfig::X;
    this->x_p = ElGamalNTLConfig::X_p;
    // std::cout << "x_p: " << this->x_p << std::endl;
    this->h = ElGamalNTLConfig::Y;
    this->h_p = ElGamalNTLConfig::Y_p;
    // std::cout << "h_p: " << this->h_p << std::endl;
    this->k = ElGamalNTLConfig::K;
    this->k_p = ElGamalNTLConfig::K_p;
    // std::cout << "k_p: " << this->k_p << std::endl;
    // std::cout << "ElGamal_parallel_ntl constructor" << std::endl;
    this->g_pow_k = ElGamalNTLConfig::GPowK;
    // std::cout << "g_pow_k: " << this->g_pow_k << std::endl;
    this->h_pow_k = ElGamalNTLConfig::YPowK;
    // std::cout << "h_pow_k: " << this->h_pow_k << std::endl;
     ZZ_p::init(ElGamalNTLConfig::P);
     this->total_chunks = (this->data_size + this->chunk_size - 1) / this->chunk_size;
     this->batch_size_encrypt = (this->total_chunks + this->num_threads - 1) / this->num_threads;
     this->batch_size_total_encrypt = this->batch_size_encrypt * this->chunk_size;
     this->logger = DurationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
     this->buffer = new unsigned char[this->per_ciphertext_size];
     this->num_pairs = ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS / (this->per_ciphertext_size * 2);
    //  this->thread_compute = new pthread_t[this->num_threads];
    this->threads.resize(this->num_threads);
    this->thread_args.resize(this->num_threads);
    this->thread_args_deserialize.resize(this->num_threads);
}

ElGamal_parallel_ntl::~ElGamal_parallel_ntl() {
    // std::cout << "ElGamal_parallel_ntl destructor" << std::endl;
    // delete[] this->buffer;
    // delete[] this->thread_compute;
}
void ElGamal_parallel_ntl::set_thread_affinity(std::thread& thread, int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    int rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
}


// useless function
void ElGamal_parallel_ntl::GenerateParams(long bitLength, long seed) {
    // std::cout << "Generating ElGamal parameters..." << std::endl;
    SetSeed(ZZ(seed));
    GenPrime(this->p, bitLength);
    this->g = ZZ(2);
    this->x = RandomBnd(this->p - 1);
    PowerMod(this->h, this->g, this->x, this->p);
    // std::cout << "Parameters generated: p = " << this->p << ", g = " << this->g << ", x = " << this->x << ", h = " << this->h << std::endl;
}

ZZ ElGamal_parallel_ntl::vector_to_ZZ(const std::vector<char> &data) {
    ZZ z;
    // Convert the entire data vector, including leading zeros
    NTL::ZZFromBytes(z, reinterpret_cast<const unsigned char*>(data.data()), data.size());
    return z;
}
ZZ_p ElGamal_parallel_ntl::vector_to_ZZ_p(const std::vector<char> &data) {
    // ZZ z = vector_to_ZZ(data);
    return conv<ZZ_p>(vector_to_ZZ(data));
}
std::vector<char> ElGamal_parallel_ntl::ZZ_to_vector(const ZZ &zz_data, size_t original_size) {
    std::vector<char> data(original_size, 0);
    BytesFromZZ(reinterpret_cast<unsigned char*>(data.data()), zz_data, original_size);
    return data;
}

std::vector<char> ElGamal_parallel_ntl::ZZ_p_to_vector(const ZZ_p &zz_data, size_t original_size) {
    return ZZ_to_vector(rep(zz_data), original_size);
}

void ElGamal_parallel_ntl::ZZ_p_to_bytes(unsigned char* buffer, size_t buffer_size, const ZZ_p& value) {
    // NTL::ZZ temp = conv<ZZ>(value); // Convert ZZ_p to ZZ
    // NTL::BytesFromZZ(buffer, temp, buffer_size); // Fill the buffer directly
    NTL::BytesFromZZ(buffer, rep(value), buffer_size); // Fill the buffer directly
}




// std::pair<ZZ, size_t> ElGamal_parallel_ntl::vector_to_ZZ(const std::vector<char> &data) {
//     size_t leading_zeros = 0;
//     // Skip leading zeros
//     while (leading_zeros < data.size() && data[leading_zeros] == 0) {
//         leading_zeros++;
//     }
//     size_t num_bytes = data.size() - leading_zeros;
//     ZZ z;
//     if (num_bytes > 0) {
//          NTL::ZZFromBytes(z, reinterpret_cast<const unsigned char*>(&data[leading_zeros]), num_bytes);
//     } else {
//         z = ZZ(0);
//     }
//     return std::make_pair(z, leading_zeros);
// }

// std::pair<ZZ_p, size_t> ElGamal_parallel_ntl::vector_to_ZZ_p(const std::vector<char> &data) {
//     auto [z, leading_zeros] = vector_to_ZZ(data);
//     // ZZ_p z_p = conv<ZZ_p>(z);
//     return std::make_pair(conv<ZZ_p>(z), leading_zeros);
// }

// std::vector<char> ElGamal_parallel_ntl::ZZ_to_vector(const ZZ &zz_data, size_t original_size, size_t leading_zeros) {
//     std::vector<char> data(original_size, 0);
//     long num_bytes = NumBytes(zz_data);
//     if (num_bytes > original_size - leading_zeros) num_bytes = original_size - leading_zeros;
//     BytesFromZZ(reinterpret_cast<unsigned char*>(&data[leading_zeros]), zz_data, num_bytes);
//     return data;
// }

// std::vector<char> ElGamal_parallel_ntl::ZZ_p_to_vector(const ZZ_p &zz_data, size_t original_size, size_t leading_zeros) {
//     return ZZ_to_vector(rep(zz_data), original_size, leading_zeros);
// }

// std::pair<ZZ, size_t> ElGamal_parallel_ntl::vector_to_ZZ(const std::vector<char> &data) {
//     ZZ z;
//     size_t leading_zeros = 0;
//     // skip leading zeros
//     size_t i = 0;
//     while (i < data.size() && data[i] == 0) {
//         leading_zeros++;
//         i++;
//     }
//     // convert the reamining data to ZZ
//     for (; i < data.size(); ++i) {
//         z <<= 8;
//         z += static_cast<unsigned char>(data[i]);
//     }
//     return std::make_pair(z, leading_zeros);
// }

// std::pair<ZZ_p, size_t> ElGamal_parallel_ntl::vector_to_ZZ_p(const std::vector<char> &data) {
//     auto [z, leading_zeros] = vector_to_ZZ(data);
//     return std::make_pair(conv<ZZ_p>(z), leading_zeros);
// }

// std::vector<char> ElGamal_parallel_ntl::ZZ_to_vector(const ZZ &zz_data, size_t original_size) {
//     std::vector<char> data(original_size, 0);
//     ZZ z = zz_data;
//     // size_t i = original_size;
//     // Fill in the data from least significant byte
//     while (z != 0 && original_size > 0) {
//         data[--original_size] = static_cast<char>(to_ulong(z & 0xFF)); // Extract the least significant byte
//         z >>= 8;  // Shift right by 8 bits
//     }
//     return data;
// }

// std::vector<char> ElGamal_parallel_ntl::ZZ_p_to_vector(const ZZ_p &zz_data, size_t original_size) {
//     // ZZ z = rep(zz_data);
//     // return ZZ_to_vector(z, original_size);
//     return ZZ_to_vector(rep(zz_data), original_size);
// }
// /*
// Function to encrypt a single block
// Name: EncryptBlock
// Input: message (ZZ)
// Output: pair of ZZ
// */
std::pair<ZZ, ZZ> ElGamal_parallel_ntl::EncryptBlock(const ZZ &message) {
    // std::cout << "Encrypting a single block..." << std::endl;

    // // Convert data to ZZ, ignore leading zeros
    // auto [message, leading_zeros] = vector_to_ZZ(data);
    // Generate a random k, which must be non-zero and less than p - 1
    ZZ k;
    do {
        k = RandomBnd(this->p - 1);
    } while (k == 0);

    ZZ c1, c2;
    PowerMod(c1, g, k, p); // c1 = g^k mod p
    ZZ s;
    PowerMod(s, h, k, p); // s = h^k mod p
    MulMod(c2, message, s, p); // c2 = m * s mod p
    return std::make_pair(c1, c2);
}
// /*
// Function to encrypt a single block  
// Name: EncryptBlock
// Input: message (ZZ_p)
// Output: pair of ZZ_p
// */

std::pair<ZZ_p, ZZ_p> ElGamal_parallel_ntl::EncryptBlock(const ZZ_p &message) {
    // // std::cout << "Encrypting a single block..." << std::endl;
    // ZZ_p c1, c2;
    // // c1 = power(g_p, k);
    // c1 = this->g_pow_k;
    // // c2 = message * power(h_p, k);
    // c2 = message * this->h_pow_k;
    // // std::cout << "c1: " << c1 << std::endl;
    // return std::make_pair(c1, c2);
    return {this->g_pow_k, message * this->h_pow_k};
}


void ElGamal_parallel_ntl::ParallelEncrypt(const std::vector<char>& data, std::vector<std::pair<ZZ, ZZ>>& ciphertexts) {
    // std::cout << "Starting parallel encryption..." << std::endl;
    ciphertexts.clear();
    size_t i = 0; 
    std::vector<std::future<std::pair<ZZ, ZZ>>> futures;

    // Encrypt the data in chunks using multiple threads
    while (i < data.size()) {
        size_t end_index = std::min(i + this->chunk_size, data.size());
        std::vector<char> chunk_data(data.begin() + i, data.begin() + end_index);
        i += this->chunk_size;
        // Launch asynchronous task to encrypt the chunk
        futures.push_back(
            std::async(
                std::launch::async, [this, chunk_data]() {
                    auto message = vector_to_ZZ(chunk_data);
                    return EncryptBlock(message);
                }
            )
        );
        
        // Manage thread pool size
        if (futures.size() >= this->num_threads) {
            ciphertexts.push_back(futures.front().get());
            futures.erase(futures.begin());
        }

    }

    // Retrieve the remaining futures
    for (auto& future : futures) {
        ciphertexts.push_back(future.get());
    }
    // std::cout << "Parallel encryption completed" << std::endl;
    
}

// void ElGamal_parallel_ntl::ParallelEncrypt(const std::vector<char>& data, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     // std::cout << "Starting parallel encryption..." << std::endl;
//     ciphertexts.clear();
//     size_t i = 0; 
//     std::vector<std::future<std::pair<ZZ_p, ZZ_p>>> futures;

//     // Encrypt the data in chunks using multiple threads
//     while (i < data.size()) {
//         size_t end_index = std::min(i + this->chunk_size, data.size());
//         std::vector<char> chunk_data(data.begin() + i, data.begin() + end_index);
//         i += this->chunk_size;
//         // Launch asynchronous task to encrypt the chunk
//         futures.push_back(
//             std::async(
//                 std::launch::async, [this, chunk_data]() {
//                     ZZ_p::init(ElGamalNTLConfig::P);  // Initialize ZZ_p
//                     auto [message, _] = this->vector_to_ZZ_p(chunk_data);
//                     return EncryptBlock(message);
//                 }
//             )
//         );
        
//         // Manage thread pool size
//         if (futures.size() >= this->num_threads) {
//             ciphertexts.push_back(futures.front().get());
//             futures.erase(futures.begin());
//         }

//     }

//     // Retrieve the remaining futures
//     for (auto& future : futures) {
//         ciphertexts.push_back(future.get());
//     }
    
// }

void ElGamal_parallel_ntl::ParallelEncrypt(const std::vector<char>& data, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    ciphertexts.clear();
     std::vector<std::future<std::vector<std::pair<ZZ_p, ZZ_p>>>> futures;
    // size_t total_chunks = (data_size + chunk_size - 1) / chunk_size;
    // this->batch_size_encrypt = (total_chunks + num_threads - 1) / num_threads; // Ceiling division
    // this->batch_size_total_encrypt = this->batch_size_encrypt * this->chunk_size;
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < data.size(); i += this->batch_size_total_encrypt) {
        ElGamalNTLConfig::TYPE_BATCH_SIZE end_index = std::min(i + this->batch_size_total_encrypt, static_cast<ElGamalNTLConfig::TYPE_BATCH_SIZE>(data.size()));
        futures.push_back(
            std::async(
                std::launch::async, [this, &data, i, end_index]() {
                    ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
                    std::vector<std::pair<ZZ_p, ZZ_p>> batch_ciphertexts;
                    for (ElGamalNTLConfig::TYPE_BATCH_SIZE j = i; j < end_index; j += this->chunk_size) {
                        ElGamalNTLConfig::TYPE_BATCH_SIZE end_chunk_index = std::min(j + this->chunk_size, end_index);
                        std::vector<char> chunk_data(data.begin() + j, data.begin() + end_chunk_index);
                        auto message = this->vector_to_ZZ_p(chunk_data);
                        batch_ciphertexts.emplace_back(EncryptBlock(message));
                    }
                    return batch_ciphertexts;
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
           auto batch_ciphertexts = futures.front().get();
           ciphertexts.insert(ciphertexts.end(), batch_ciphertexts.begin(), batch_ciphertexts.end());
           futures.erase(futures.begin());
        }
    }

    // Retrieve the remaining futures
    for (auto& future : futures) {
        auto batch_ciphertexts = future.get();
        ciphertexts.insert(ciphertexts.end(), batch_ciphertexts.begin(), batch_ciphertexts.end());
    }
}


ZZ ElGamal_parallel_ntl::DecryptBlock(const std::pair<ZZ, ZZ> &ciphertext) {
    ZZ c1 = ciphertext.first;
    ZZ c2 = ciphertext.second;
    ZZ s;
    PowerMod(s, c1, x, p); // s = c1^x mod p
    ZZ s_inv;
    InvMod(s_inv, s, p); // s_inv = s^-1 mod p
    ZZ message;
    MulMod(message, c2, s_inv, p); // m = c2 * s_inv mod p
    return message;
}

ZZ_p ElGamal_parallel_ntl::DecryptBlock(const std::pair<ZZ_p, ZZ_p> &ciphertext) {
    // ZZ_p c1 = ciphertext.first;
    // ZZ_p c2 = ciphertext.second;
    // ZZ_p s = power(ciphertext.first, x_p);
    // ZZ_p s_inv = inv(power(ciphertext.first, x_p));
    // ZZ_p message = ciphertext.second * inv(power(ciphertext.first, x));
    
    // return ciphertext.second * inv(power(ciphertext.first, x));
    #if USE_ASSERT
    assert(inv(power(ciphertext.first, x)) == this->g_pow_k_x_inv);
    #endif
    return ciphertext.second * this->g_pow_k_x_inv;
    // Option 1: Use negative exponent
    // return ciphertext.second * power(ciphertext.first, -x);
    // Option 2: Use division
    // return ciphertext.second / power(ciphertext.first, x);
}
void ElGamal_parallel_ntl::ParallelDecrypt(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts, std::vector<char>& data) {
    #if MULTI_THREAD_SWITCH
    // std::cout << "Starting parallel decryption..." << std::endl;
    data.clear();
    // size_t i = 0;
    this->i_parallel_decrypt = 0;
    this->c_index = 0;
    std::vector<std::future<std::vector<char>>> futures;

    // Decrypt each chunk in parallel
    while (this->i_parallel_decrypt + this->chunk_size <= this->data_size) {
      const std::pair<ZZ, ZZ>& ciphertext = ciphertexts[this->c_index++];
        this->i_parallel_decrypt += this->chunk_size;
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertext]() {
                    ZZ decrypted_message = DecryptBlock(ciphertext);
                    return ZZ_to_vector(decrypted_message, chunk_size);
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            // std::vector<char> result = futures.front().get();
            std::vector<char> result = std::move(futures.front().get());
            // data.insert(data.end(), result.begin(), result.end());
            std::copy(result.begin(), result.end(), std::back_inserter(data));
            futures.erase(futures.begin());
        }
    }

    if (this->i_parallel_decrypt < this->data_size) {
        const std::pair<ZZ, ZZ>& ciphertext = ciphertexts[this->c_index++];
        size_t remaining_size = this->data_size - this->i_parallel_decrypt;
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertext, &remaining_size]() {
                    ZZ decrypted_message = DecryptBlock(ciphertext);
                    return ZZ_to_vector(decrypted_message, remaining_size);
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            // std::vector<char> result = futures.front().get();
            std::vector<char> result = std::move(futures.front().get());
            // data.insert(data.end(), result.begin(), result.end());
            std::copy(result.begin(), result.end(), std::back_inserter(data));
            futures.erase(futures.begin());
        }

    }

    // Retrieve the remaining futures
    for (auto& future : futures) {
        // std::vector<char> result = future.get();
        std::vector<char> result = std::move(future.get());
        // data.insert(data.end(), result.begin(), result.end());
        std::copy(result.begin(), result.end(), std::back_inserter(data));
    }
    #else
        for (size_t i = 0; i < ciphertexts.size(); ++i) {
            std::copy(
                ZZ_to_vector(DecryptBlock(ciphertexts[i]), (i < ciphertexts.size() - 1) ? this->chunk_size : (this->data_size - i * this->chunk_size)).begin(),
                ZZ_to_vector(DecryptBlock(ciphertexts[i]), (i < ciphertexts.size() - 1) ? this->chunk_size : (this->data_size - i * this->chunk_size)).end(),
                data.begin() + i * this->chunk_size
            );
        }
    #endif
}

// void ElGamal_parallel_ntl::ParallelDecrypt(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, std::vector<char>& data) {
//     // std::cout << "Starting parallel decryption..." << std::endl;
//     data.clear();
//     size_t i = 0;
//     size_t c_index = 0;
//     std::vector<std::future<std::vector<char>>> futures;

//     // Decrypt each chunk in parallel
//     while (i + this->chunk_size <= this->data_size) {
//       const std::pair<ZZ_p, ZZ_p>& ciphertext = ciphertexts[c_index++];
//         i += this->chunk_size;
//         futures.push_back(
//             std::async(
//                 std::launch::async, [this, &ciphertext]() {
//                     ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
//                     ZZ_p decrypted_message = DecryptBlock(ciphertext);
//                     return ZZ_p_to_vector(decrypted_message, chunk_size);
//                 }
//             )
//         );

//         // Ensure the number of active threads does not exceed the limit
//         if (futures.size() >= this->num_threads) {
//             std::vector<char> result = futures.front().get();
//             data.insert(data.end(), result.begin(), result.end());
//             futures.erase(futures.begin());
//         }
//     }

//     if (i < this->data_size) {
//         const std::pair<ZZ_p, ZZ_p>& ciphertext = ciphertexts[c_index++];
//         size_t remaining_size = this->data_size - i;
//         futures.push_back(
//             std::async(
//                 std::launch::async, [this, &ciphertext, &remaining_size]() {
//                     ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
//                     ZZ_p decrypted_message = DecryptBlock(ciphertext);
//                     return ZZ_p_to_vector(decrypted_message, remaining_size);
//                 }
//             )
//         );

//         // Ensure the number of active threads does not exceed the limit
//         if (futures.size() >= this->num_threads) {
//             std::vector<char> result = futures.front().get();
//             data.insert(data.end(), result.begin(), result.end());
//             futures.erase(futures.begin());
//         }

//     }

//     // Retrieve the remaining futures
//     for (auto& future : futures) {
//         std::vector<char> result = future.get();
//         data.insert(data.end(), result.begin(), result.end());
//     }
//     // std::cout << "Parallel decryption completed" << std::endl;
// }
void ElGamal_parallel_ntl::ParallelDecrypt(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, std::vector<char>& data) {
    // Preallocate data to the correct size
    data.resize(this->data_size);
    #if MULTI_THREAD_SWITCH
    std::vector<std::future<void>> futures;
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < ciphertexts.size(); i += this->batch_size_decrypt) {
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertexts, &data, i]() {
                    ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
                    for (int j = i; j < i + this->batch_size_decrypt && j < ciphertexts.size(); j++) {
                        ZZ_p decrypted_message = DecryptBlock(ciphertexts[j]);
                        // Calculate the size of the decrypted chunk for this ciphertext
                        size_t chunk_size = this->chunk_size;
                        if ((j + 1) * this->chunk_size > this->data_size) {
                            chunk_size = this->data_size - j * this->chunk_size;
                        }
                        std::vector<char> decrypted_chunk = ZZ_p_to_vector(decrypted_message, chunk_size);
                        std::copy(decrypted_chunk.begin(), decrypted_chunk.end(), data.begin() + j * this->chunk_size);
                    }
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            futures.front().get();
            futures.erase(futures.begin());
        }
    }

    // Ensure all remaining futures are completed
    for (auto& future : futures) {
        future.get();
    }
    #else
   // Iterate over each ciphertext and decrypt sequentially
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < ciphertexts.size(); ++i) {
        if (i == 0) {
            this->g_pow_k_x_inv = inv(power(ciphertexts[i].first, x));
        }
        // Simplest Version
        // this->decrypted_chunk = std::move(ZZ_p_to_vector(DecryptBlock(ciphertexts[i]), (i == ciphertexts.size() - 1) ? (this->data_size - i * this->chunk_size) : this->chunk_size));
        // std::copy(this->decrypted_chunk.begin(), this->decrypted_chunk.end(), data.begin() + i * this->chunk_size);
       #if LOG_BREAKDOWN_COST_READ_PATH_FURTHER
       logger.startTiming(this->LogSingleChunkDecrypt);
       #endif
       this->decrypted_message_p = std::move(DecryptBlock(ciphertexts[i]));
       #if LOG_BREAKDOWN_COST_READ_PATH_FURTHER
        logger.stopTiming(this->LogSingleChunkDecrypt);
        logger.writeToFile();
        logger.startTiming(this->LogConvertToVector);
       #endif   
        this->chunk_size_decrypt_p = (i == ciphertexts.size() - 1) ? (this->data_size - i * this->chunk_size) : this->chunk_size;
        this->decrypted_chunk = ZZ_p_to_vector(decrypted_message_p, this->chunk_size_decrypt_p);
        #if LOG_BREAKDOWN_COST_READ_PATH_FURTHER
        logger.stopTiming(this->LogConvertToVector);
        logger.writeToFile();
        logger.startTiming(this->LogCopyBlock);
        #endif
        // std::copy(this->decrypted_chunk.begin(), this->decrypted_chunk.end(), data.begin() + i * this->chunk_size);
        std::memcpy(data.data() + i * this->chunk_size, this->decrypted_chunk.data(), this->chunk_size_decrypt_p);
        #if LOG_BREAKDOWN_COST_READ_PATH_FURTHER
        logger.stopTiming(this->LogCopyBlock);
        logger.writeToFile();
        #endif
    }
    #endif 
}


std::pair<ZZ, ZZ> ElGamal_parallel_ntl::ReRandomizeBlock(const std::pair<ZZ, ZZ> &ciphertext) {
    ZZ c1 = ciphertext.first;
    ZZ c2 = ciphertext.second;
    ZZ k;
    do {
        k = RandomBnd(this->p - 1);
    } while (k == 0);

    auto rerandomize_component = [this, &k](const ZZ& component, const ZZ& base) {
        ZZ new_component;
        PowerMod(new_component, base, k, this->p); // new_component = base^k mod p
        MulMod(new_component, component, new_component, this->p); // new_component = component * new_component mod p
        return new_component;
    };

    ZZ c1_new = rerandomize_component(c1, this->g);
    ZZ c2_new = rerandomize_component(c2, this->h);
    
    return std::make_pair(c1_new, c2_new);
}

std::pair<ZZ_p, ZZ_p> ElGamal_parallel_ntl::ReRandomizeBlock(const std::pair<ZZ_p, ZZ_p> &ciphertext) noexcept {
    return {
        ciphertext.first * this->g_pow_k,  // Efficient multiplication of precomputed g^k
        ciphertext.second * this->h_pow_k  // Efficient multiplication of precomputed h^k
    };
}

void ElGamal_parallel_ntl::ReRandomizeBlock(std::pair<ZZ_p, ZZ_p>& ciphertext) noexcept {
    ciphertext.first *= this->g_pow_k;   // Efficient in-place multiplication
    ciphertext.second *= this->h_pow_k;  // Efficient in-place multiplication
}





void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ, ZZ>>& ciphertexts) {
    #if MULTI_THREAD_SWITCH
    // std::cout << "Starting parallel re-randomization..." << std::endl;
    std::vector<std::future<void>> futures;
    for (auto& ciphertext : ciphertexts) {
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertext]() {
                    ciphertext = ReRandomizeBlock(ciphertext);
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            futures.front().get();
            futures.erase(futures.begin());
        }
    }
    // Ensure all remaining futures are completed
    for (auto& future : futures) {
        future.get();
    }
    // std::cout << "Parallel re-randomization completed" << std::endl;
    #else
    for (auto& ciphertext : ciphertexts) {
        ciphertext = ReRandomizeBlock(ciphertext);
    }
    #endif

}

// void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     std::vector<std::future<void>> futures;
//     for (auto& ciphertext : ciphertexts) {
//         futures.push_back(
//             std::async(
//                 std::launch::async, [this, &ciphertext]() {
//                     ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
//                     ciphertext = ReRandomizeBlock(ciphertext);
//                 }
//             )
//         );

//         // Ensure the number of active threads does not exceed the limit
//         if (futures.size() >= this->num_threads) {
//             futures.front().get();
//             futures.erase(futures.begin());
//         }
//     }
//     // Ensure all remaining futures are completed
//     for (auto& future : futures) {
//         future.get();
//     }
// }

// void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     for (auto& ciphertext : ciphertexts) {
//         // ciphertext = ReRandomizeBlock(ciphertext);
//         ciphertext.first *= this->g_pow_k;
//         ciphertext.second *= this->h_pow_k;
//     }
// }
#if MULTI_THREAD_BASIC_THREAD_POOL
void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    BasicThreadPool pool(this->num_threads);  // Initialize thread pool with desired number of threads

    // Define the batch processing in the lambda function
    pool.exec_range(ciphertexts.size(), [&](long first, long last) {
        ZZ_p::init(ElGamalNTLConfig::P);  // Set ZZ_p context locally for each thread

        for (long i = first; i < last; ++i) {
            ReRandomizeBlock(ciphertexts[i]);
        }
    });
}



#else
void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    #if MULTI_THREAD_RERANDOMIZE_SWITCH 
    #if MULTI_THREAD_FUTURE_VERSION
    std::vector<std::future<void>> futures_rerandomize;
    this->batch_size_rerandomize = ciphertexts.size() / this->num_threads;
    // std::cout << "batch_size_rerandomize: " << this->batch_size_rerandomize << std::endl;
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < ciphertexts.size(); i += this->batch_size_rerandomize) {
        futures_rerandomize.push_back(
            std::async(
                std::launch::async, [this, &ciphertexts, i]() {
                    ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
                    for (int j = i; j < i + this->batch_size_rerandomize && j < ciphertexts.size(); j++) {
                        // ciphertexts[j] = ReRandomizeBlock(ciphertexts[j]);
                        // ciphertexts[j] = std::move(ReRandomizeBlock(ciphertexts[j]));
                        ReRandomizeBlock(ciphertexts[j]);
                    }
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures_rerandomize.size() >= this->num_threads) {
            futures_rerandomize.front().get();
            futures_rerandomize.erase(futures_rerandomize.begin());
        }
    }

    // Ensure all remaining futures are completed
    for (auto& future : futures_rerandomize) {
        future.get();
    }
    #elif MULTI_THREAD_THREAD_VERSION
    this->batch_size_rerandomize = (ciphertexts.size() + this->num_threads - 1) / this->num_threads;
    std::vector<std::thread> threads;
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < ciphertexts.size(); i += this->batch_size_rerandomize) {
        threads.emplace_back(
            [this, &ciphertexts, i]() {
                ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
                for (int j = i; j < i + this->batch_size_rerandomize && j < ciphertexts.size(); j++) {
                    ReRandomizeBlock(ciphertexts[j]);
                }
            }
        );
        // Set CPU affinity for the thread
        set_thread_affinity(threads.back(), i / this->batch_size_rerandomize % this->num_threads);
    }
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    #else
    #if USE_COUT
    std::cout << "The size of the ciphertexts: " << ciphertexts.size() << std::endl;
    #endif
    this->batch_size_rerandomize = (ciphertexts.size() + this->num_threads - 1) / this->num_threads;
    this->num_batches = (ciphertexts.size() + this->batch_size_rerandomize - 1) / this->batch_size_rerandomize;
   
    for (size_t batch = 0; batch < this->num_batches; ++batch) {
        this->startIdx = batch * this->batch_size_rerandomize;
        this->endIdx = this->startIdx + this->batch_size_rerandomize;
        if (this->endIdx > ciphertexts.size()) {
            this->endIdx = ciphertexts.size();
        }

        thread_args[batch] = { &ciphertexts, this->startIdx, this->endIdx, this->g_pow_k, this->h_pow_k, batch % this->num_threads };
        pthread_create(&threads[batch], nullptr, thread_func, (void*)&thread_args[batch]);
    }
    for (size_t batch = 0; batch <this->num_batches; ++batch) {
        #if PRINT_BREAKDOWN_COST_RERANDOMIZE
        auto start = std::chrono::high_resolution_clock::now();
        #endif
        pthread_join(threads[batch], nullptr);
        #if PRINT_BREAKDOWN_COST_RERANDOMIZE
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "ReRandomizeElement took " << duration << " nanoseconds." << std::endl;
        #endif
    }



    #endif 
    #else
    for (auto& ciphertext : ciphertexts) {
        #if PRINT_BREAKDOWN_COST_RERANDOMIZE
        auto start = std::chrono::high_resolution_clock::now();
        #endif
        ReRandomizeBlock(ciphertext);
        #if PRINT_BREAKDOWN_COST_RERANDOMIZE
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "ReRandomizeElement took " << duration << " nanoseconds." << std::endl;
        #endif
    }
    #endif
}
#endif

// // The version of the customized ThreadPool class
// void ElGamal_parallel_ntl::ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts ) {
//     this->batch_size_rerandomize = ciphertexts.size() / this->num_threads;
//     ThreadPool pool(this->num_threads);
//     for (ElGamalNTLConfig::TYPE_BATCH_SIZE i = 0; i < ciphertexts.size(); i += this->batch_size_rerandomize) {
//         pool.enqueue(
//             [this, &ciphertexts, i]() {
//                 // ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
//                 for (int j = i; j < i + this->batch_size_rerandomize && j < ciphertexts.size(); j++) {
//                     ciphertexts[j] = ReRandomizeBlock(ciphertexts[j]);
//                 }
//             }
//         );
//     }

//     pool.wait_until_done();
// }


std::pair<ZZ, ZZ> ElGamal_parallel_ntl::MultiplyCiphertexts(const std::pair<ZZ, ZZ> &ciphertext1, const std::pair<ZZ, ZZ> &ciphertext2) {
    ZZ c1_new, c2_new;
    MulMod(c1_new, ciphertext1.first, ciphertext2.first, this->p); // c1_new = c1 * c1 mod p
    MulMod(c2_new, ciphertext1.second, ciphertext2.second, this->p); // c2_new = c2 * c2 mod p
    return std::make_pair(c1_new, c2_new);
}

std::pair<ZZ_p, ZZ_p> ElGamal_parallel_ntl::MultiplyCiphertexts(const std::pair<ZZ_p, ZZ_p> &ciphertext1, const std::pair<ZZ_p, ZZ_p> &ciphertext2) {
    // ZZ_p c1_new = ciphertext1.first * ciphertext2.first;
    // ZZ_p c2_new = ciphertext1.second * ciphertext2.second;
    // return std::make_pair(c1_new, c2_new);
    return {ciphertext1.first * ciphertext2.first, ciphertext1.second * ciphertext2.second};
}

void ElGamal_parallel_ntl::ParallelMultiplyCiphertexts(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts1, const std::vector<std::pair<ZZ, ZZ>>& ciphertexts2, std::vector<std::pair<ZZ, ZZ>>& result) {
    assert(ciphertexts1.size() == ciphertexts2.size());
    result.clear();
    std::vector<std::future<std::pair<ZZ, ZZ>>> futures;

    for (size_t i = 0; i < ciphertexts1.size(); i++) {
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertexts1, &ciphertexts2, i]() {
                    return MultiplyCiphertexts(ciphertexts1[i], ciphertexts2[i]);
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            result.push_back(futures.front().get());
            futures.erase(futures.begin());
        }
    }
    // Retrieve the remaining futures
    for (auto& future : futures) {
        result.push_back(future.get());
    }
}

void ElGamal_parallel_ntl::ParallelMultiplyCiphertexts(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts1, const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts2, std::vector<std::pair<ZZ_p, ZZ_p>>& result) {
    #if USE_ASSERT
    assert(ciphertexts1.size() == ciphertexts2.size());
    #endif
    #if MULTI_THREAD_SWITCH
    result.clear();
    std::vector<std::future<std::pair<ZZ_p, ZZ_p>>> futures;

    for (size_t i = 0; i < ciphertexts1.size(); i++) {
        futures.push_back(
            std::async(
                std::launch::async, [this, &ciphertexts1, &ciphertexts2, i]() {
                    ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p
                    return MultiplyCiphertexts(ciphertexts1[i], ciphertexts2[i]);
                }
            )
        );

        // Ensure the number of active threads does not exceed the limit
        if (futures.size() >= this->num_threads) {
            result.push_back(futures.front().get());
            futures.erase(futures.begin());
        }
    }
    // Retrieve the remaining futures
    for (auto& future : futures) {
        result.push_back(future.get());
    }
    #else
    result.resize(ciphertexts1.size());
    for (size_t i = 0; i < ciphertexts1.size(); i++) {
        result[i] = MultiplyCiphertexts(ciphertexts1[i], ciphertexts2[i]);
    }
    #endif
}

void ElGamal_parallel_ntl::SerializeCiphertexts(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts, std::vector<char>& serializedData) {
    serializedData.clear();
    for (const auto& ciphertext : ciphertexts) {
        // Serialize the first ZZ in the pair
        size_t first_size = NumBytes(ciphertext.first);
        serializedData.insert(serializedData.end(), reinterpret_cast<const char*>(&first_size), reinterpret_cast<const char*>(&first_size) + sizeof(first_size));
        auto first_part = ZZ_to_vector(ciphertext.first, first_size);
        serializedData.insert(serializedData.end(), first_part.begin(), first_part.end());

        // Serialize the second ZZ in the pair
        size_t second_size = NumBytes(ciphertext.second);
        serializedData.insert(serializedData.end(), reinterpret_cast<const char*>(&second_size), reinterpret_cast<const char*>(&second_size) + sizeof(second_size));
        auto second_part = ZZ_to_vector(ciphertext.second, second_size);
        serializedData.insert(serializedData.end(), second_part.begin(), second_part.end());
    }
}

void ElGamal_parallel_ntl::SerializeCiphertexts(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, std::vector<char>& serializedData) {
    // serializedData.clear();
    // for (const auto& ciphertext : ciphertexts) {
    //     // Serialize the first ZZ_p in the pair
    //     // size_t first_size = NumBytes(conv<ZZ>(ciphertext.first));
    //     // std::cout << "first_size: " << first_size << std::endl;
    //     // serializedData.insert(serializedData.end(), reinterpret_cast<const char*>(&first_size), reinterpret_cast<const char*>(&first_size) + sizeof(first_size));
    //     auto first_part = ZZ_p_to_vector(ciphertext.first, this->per_ciphertext_size);
    //     serializedData.insert(serializedData.end(), first_part.begin(), first_part.end());

    //     // Serialize the second ZZ_p in the pair
    //     // size_t second_size = NumBytes(conv<ZZ>(ciphertext.second));
    //     // std::cout << "second_size: " << second_size << std::endl;
    //     // serializedData.insert(serializedData.end(), reinterpret_cast<const char*>(&second_size), reinterpret_cast<const char*>(&second_size) + sizeof(second_size));
        // auto second_part = ZZ_p_to_vector(ciphertext.second, this->per_ciphertext_size);
    //     serializedData.insert(serializedData.end(), second_part.begin(), second_part.end());
    // }
    serializedData.clear();
    serializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS); // Resize to exact size needed

    // unsigned char buffer[this->per_ciphertext_size];
    // size_t offset = 0;
    auto it = serializedData.begin();

    for (const auto& ciphertext : ciphertexts) {
        // Serialize the first part directly into serializedData
        // ZZ_p_to_bytes(buffer, this->per_ciphertext_size, ciphertext.first);
        // NTL::BytesFromZZ(buffer, rep(ciphertext.first), this->per_ciphertext_size);
        // // std::copy(buffer, buffer + this->per_ciphertext_size, serializedData.begin() + offset);
        // std::memcpy(serializedData.data() + offset, buffer, this->per_ciphertext_size);
        // offset += this->per_ciphertext_size;
        NTL::BytesFromZZ(reinterpret_cast<unsigned char*>(&(*it)), rep(ciphertext.first), this->per_ciphertext_size);
        std::advance(it, this->per_ciphertext_size);

        // Serialize the second part directly into serializedData
        // // ZZ_p_to_bytes(buffer, this->per_ciphertext_size, ciphertext.second);
        // NTL::BytesFromZZ(buffer, rep(ciphertext.second), this->per_ciphertext_size);
        // // std::copy(buffer, buffer + this->per_ciphertext_size, serializedData.begin() + offset);
        // std::memcpy(serializedData.data() + offset, buffer, this->per_ciphertext_size);
        // offset += this->per_ciphertext_size;
        // Write the second part directly to serializedData
        NTL::BytesFromZZ(reinterpret_cast<unsigned char*>(&(*it)), rep(ciphertext.second), this->per_ciphertext_size);
        std::advance(it, this->per_ciphertext_size);
    }

}

void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ, ZZ>>& ciphertexts) {
    ciphertexts.clear();
    size_t offset = 0;

    while (offset < serializedData.size()) {
        size_t first_size;
        std::memcpy(&first_size, serializedData.data() + offset, sizeof(first_size));
        offset += sizeof(first_size);

        ZZ first_part;
        NTL::ZZFromBytes(first_part, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), first_size);
        offset += first_size;

        size_t second_size;
        std::memcpy(&second_size, serializedData.data() + offset, sizeof(second_size));
        offset += sizeof(second_size);

        ZZ second_part;
        NTL::ZZFromBytes(second_part, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), second_size);
        offset += second_size;

        // ciphertexts.emplace_back(first_part, second_part);
        ciphertexts.emplace_back(std::move(first_part), std::move(second_part));
    }
}

// void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     // ZZ_p::init(ElGamalNTLConfig::P);
//     ciphertexts.clear();
//     size_t offset = 0;
//     while (offset < serializedData.size()) {
//         ZZ temp_zz;
//         NTL::ZZFromBytes(temp_zz, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), this->per_ciphertext_size);
//         ZZ_p first_part = conv<ZZ_p>(temp_zz);
//         offset += this->per_ciphertext_size;

//         NTL::ZZFromBytes(temp_zz, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), this->per_ciphertext_size);
//         ZZ_p second_part = conv<ZZ_p>(temp_zz);
//         offset += this->per_ciphertext_size;

//         ciphertexts.emplace_back(first_part, second_part);
//     }
// }

void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    #if USE_ASSERT
    assert(serializedData.size() == ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS && "Invalid serialized data size");
    #endif
    // // num_pairs = serializedData.size() / (2 * this->per_ciphertext_size);
    // ciphertexts.clear();
    // // ciphertexts.reserve(num_pairs);  // Preallocate vector to avoid reallocations

    // data_ptr = reinterpret_cast<const unsigned char*>(serializedData.data());
    // data_end = data_ptr + serializedData.size();

    // while (data_ptr < data_end) {
    //     // Deserialize first_part
    //     NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
    //     conv(first_part_dser, temp_zz_dser);  // Convert ZZ to ZZ_p
    //     data_ptr += this->per_ciphertext_size;

    //     // Deserialize second_part
    //     NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
    //     conv(second_part_dser, temp_zz_dser);  // Convert ZZ to ZZ_p
    //     data_ptr += this->per_ciphertext_size;

    //     // Add the pair to the ciphertexts vector
    //     // ciphertexts.emplace_back(first_part_dser, second_part_dser);
    //     ciphertexts.emplace_back(std::move(first_part_dser), std::move(second_part_dser));
    // }
    #if MULTI_THREAD_DESERIALIZE_SWITCH
    // long num_pairs = serializedData.size() / (2 * this->per_ciphertext_size);
    int step = ceil((double)num_pairs / (double)this->num_threads);
    long endIdx;
    ciphertexts.resize(num_pairs);
    struct ThreadArgs {
        int startIdx;
        int endIdx;
        const std::vector<char>* serializedData;
        std::vector<std::pair<ZZ_p, ZZ_p>>* ciphertexts;
        int per_ciphertext_size;
    } thread_args[this->num_threads];
    auto thread_func = [](void* arg) -> void* {
        ThreadArgs* args = static_cast<ThreadArgs*>(arg);
        ZZ temp_zz_dser;
        ZZ_pContext context(ElGamalNTLConfig::P);
        context.restore();
        const unsigned char* data_ptr = reinterpret_cast<const unsigned char*>(args->serializedData->data());
        for (int j = args->startIdx; j < args->endIdx; ++j) {
            const unsigned char* pair_ptr = data_ptr + j * 2 * args->per_ciphertext_size;

            // Deserialize first_part
            NTL::ZZFromBytes(temp_zz_dser, pair_ptr, args->per_ciphertext_size);
            (*args->ciphertexts)[j].first = conv<ZZ_p>(temp_zz_dser);

            // Deserialize second_part
            NTL::ZZFromBytes(temp_zz_dser, pair_ptr + args->per_ciphertext_size, args->per_ciphertext_size);
            (*args->ciphertexts)[j].second = conv<ZZ_p>(temp_zz_dser);
        }
        return nullptr;
    };
    for (int i = 0, startIdx = 0; i < this->num_threads && startIdx < num_pairs; i++, startIdx += step) {
        if (startIdx + step > num_pairs) {
            endIdx = num_pairs;
        } else {
            endIdx = startIdx + step;
        }

        thread_args[i] = {startIdx, endIdx, &serializedData, &ciphertexts, this->per_ciphertext_size};
        pthread_create(&thread_compute[i], NULL, thread_func, (void*)&thread_args[i]);

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        pthread_setaffinity_np(thread_compute[i], sizeof(cpu_set_t), &cpuset);
    }

    for (int i = 0, startIdx = 0; i < this->num_threads && startIdx < num_pairs; i++, startIdx += step) {
        pthread_join(thread_compute[i], NULL);
    }

    #else
    ciphertexts.resize(num_pairs);
    data_ptr = reinterpret_cast<const unsigned char*>(serializedData.data());
    for (size_t i = 0; i < num_pairs; ++i) {
        // Deserialize first_part directly
        NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
        data_ptr += this->per_ciphertext_size;
        ciphertexts[i].first = conv<ZZ_p>(temp_zz_dser);  // Direct assignment

        // Deserialize second_part directly
        NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
        data_ptr += this->per_ciphertext_size;
        ciphertexts[i].second = conv<ZZ_p>(temp_zz_dser);  // Direct assignment
    }
    #endif
}
void ElGamal_parallel_ntl::DeserializeCiphertexts(const unsigned char* data_ptr, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    ciphertexts.resize(num_pairs);
    for (size_t i = 0; i < num_pairs; ++i) {
        // Deserialize first_part directly
        NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
        data_ptr += this->per_ciphertext_size;
        ciphertexts[i].first = conv<ZZ_p>(temp_zz_dser);  // Direct assignment

        // Deserialize second_part directly
        NTL::ZZFromBytes(temp_zz_dser, data_ptr, this->per_ciphertext_size);
        data_ptr += this->per_ciphertext_size;
        ciphertexts[i].second = conv<ZZ_p>(temp_zz_dser);  // Direct assignment
    }
}

void ElGamal_parallel_ntl::DeserializeCiphertexts(const unsigned char* data_ptr, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, ElGamalNTLConfig::TYPE_BATCH_SIZE num_pairs_to_deserialize) {
    size_t num_threads = 2;
    this->batch_size_rerandomize = (num_pairs_to_deserialize + num_threads - 1) / num_threads;
    this->num_batches = (num_pairs_to_deserialize + this->batch_size_rerandomize - 1) / this->batch_size_rerandomize;
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE batch = 0; batch < this->num_batches; ++batch) {
        this->startIdx = batch * this->batch_size_rerandomize;
        this->endIdx = this->startIdx + this->batch_size_rerandomize;
        if (this->endIdx > num_pairs_to_deserialize) {
            this->endIdx = num_pairs_to_deserialize;
        }
        this->thread_args_deserialize[batch] = { data_ptr, &ciphertexts, this->startIdx, this->endIdx, (batch % num_threads)};
        pthread_create(&this->threads[batch], nullptr, thread_func_deserialize, (void*)&this->thread_args_deserialize[batch]);
    }
    for (ElGamalNTLConfig::TYPE_BATCH_SIZE batch = 0; batch < this->num_batches; ++batch) {
        pthread_join(this->threads[batch], nullptr);
    }
}


// void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     ZZ_p::init(ElGamalNTLConfig::P);
//     ciphertexts.clear();
//     size_t offset = 0;
//     while (offset < serializedData.size()) {
//         // Read the size of the first component
//         long first_num_bytes;
//         std::memcpy(&first_num_bytes, serializedData.data() + offset, sizeof(long));
//         offset += sizeof(long);
//         // Read the bytes of the first component
//         ZZ first_zz;
//         NTL::ZZFromBytes(first_zz, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), first_num_bytes);
//         ZZ_p first_part = conv<ZZ_p>(first_zz);
//         offset += first_num_bytes;

//         // Read the size of the second component
//         long second_num_bytes;
//         std::memcpy(&second_num_bytes, serializedData.data() + offset, sizeof(long));
//         offset += sizeof(long);
//         // Read the bytes of the second component
//         ZZ second_zz;
//         NTL::ZZFromBytes(second_zz, reinterpret_cast<const unsigned char*>(serializedData.data() + offset), second_num_bytes);
//         ZZ_p second_part = conv<ZZ_p>(second_zz);
//         offset += second_num_bytes;

//         ciphertexts.emplace_back(first_part, second_part);
//     }
// }

void ElGamal_parallel_ntl::DeserializeCiphertexts(const char* dataPtr, size_t dataSize, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
    ZZ_p::init(ElGamalNTLConfig::P);
    ciphertexts.clear();
    size_t offset = 0;
    size_t perCiphertextSize = this->per_ciphertext_size * 2; // Since each ciphertext has two parts

    while (offset < dataSize) {
        // Deserialize first part
        ZZ temp_zz;
        NTL::ZZFromBytes(
            temp_zz,
            reinterpret_cast<const unsigned char*>(dataPtr + offset),
            this->per_ciphertext_size
        );
        ZZ_p first_part = conv<ZZ_p>(temp_zz);
        offset += this->per_ciphertext_size;

        // Deserialize second part
        NTL::ZZFromBytes(
            temp_zz,
            reinterpret_cast<const unsigned char*>(dataPtr + offset),
            this->per_ciphertext_size
        );
        ZZ_p second_part = conv<ZZ_p>(temp_zz);
        offset += this->per_ciphertext_size;

        // Store the ciphertext
        ciphertexts.emplace_back(first_part, second_part);
    }
}


// void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ, ZZ>>& ciphertexts) {
//     ciphertexts.clear();
//     size_t offset = 0;

//     while (offset < serializedData.size()) {
//         // Read the size of the first ZZ
//         size_t first_size;
//         std::memcpy(&first_size, serializedData.data() + offset, sizeof(first_size));
//         offset += sizeof(first_size);

//         // Deserialize the first ZZ in the pair
//         ZZ first_part;
//         first_part = ZZFromBytes(reinterpret_cast<const unsigned char*>(serializedData.data() + offset), first_size);
//         offset += first_size;

//         // Read the size of the second ZZ
//         size_t second_size;
//         std::memcpy(&second_size, serializedData.data() + offset, sizeof(second_size));
//         offset += sizeof(second_size);

//         // Deserialize the second ZZ in the pair
//         ZZ second_part;
//         second_part = ZZFromBytes(reinterpret_cast<const unsigned char*>(serializedData.data() + offset), second_size);
//         offset += second_size;

//         ciphertexts.emplace_back(first_part, second_part);
        
//     }
// }

// void ElGamal_parallel_ntl::DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts) {
//     ZZ_p::init(ElGamalNTLConfig::P);
//     ciphertexts.clear();
//     size_t offset = 0;
//     while (offset < serializedData.size()) {
//         // Read the size of the first ZZ_p
//         // size_t first_size;
//         // std::memcpy(&first_size, serializedData.data() + offset, sizeof(first_size));
//         // offset += sizeof(first_size);
//         // Deserialize the first ZZ_p in the pair
//         ZZ_p first_part;
//         first_part = conv<ZZ_p>(ZZFromBytes(reinterpret_cast<const unsigned char*>(serializedData.data() + offset), this->per_ciphertext_size));
//         offset += this->per_ciphertext_size;

//         // // Read the size of the second ZZ_p
//         // size_t second_size;
//         // std::memcpy(&second_size, serializedData.data() + offset, sizeof(second_size));
//         // offset += sizeof(second_size);

//         // Deserialize the second ZZ_p in the pair
//         ZZ_p second_part;
//         second_part = conv<ZZ_p>(ZZFromBytes(reinterpret_cast<const unsigned char*>(serializedData.data() + offset), this->per_ciphertext_size));
//         offset += this->per_ciphertext_size;

//         ciphertexts.emplace_back(first_part, second_part);
        
//     }
//     // std::cout << "End the deserialization" << std::endl;
// }

// ZZ ElGamal_parallel_ntl::ZZFromBytes(const unsigned char* data, size_t size) {
//     ZZ z;
//     for (size_t i = 0; i < size; ++i) {
//         z <<= 8;
//         z += data[i];
//     }
//     return z;
// }

// ZZ_p ElGamal_parallel_ntl::ZZ_pFromBytes(const unsigned char* data, size_t size) {
//     // ZZ_p::init(ElGamalNTLConfig::P);
//     ZZ z = ZZFromBytes(data, size);
//     return conv<ZZ_p>(z);
// }
