#ifndef CLIENT_H
#define CLIENT_H
#include "config.h"
#include "SocketCommunicator.h"
#include "Path.h"
#include "Tree.h"
#include <unordered_map>
#include "DurationLogger.h"
#include "AES_CTR.h"
#include "Block.h"  
class Client {
public:
    Client(ClientConfig::TYPE_HOST host, ClientConfig::TYPE_PORT port);
    ~Client();
    void InitConnection();
    void EnsureConnection();   
    void InitReadPath(PathConfig::TYPE_PATH_ID path_id);
    void ReadPath(BlockConfig::TYPE_BLOCK_ID block_id, PathConfig::TYPE_PATH_ID path_id);
    void InitEarlyReshuffle();
    void EarlyReshuffle(BucketConfig::TYPE_BUCKET_ID bucket_id = 0);
    void InitEviction(PathConfig::TYPE_PATH_ID path_id = 0);
    void CommunicateRootBucket();
    void Evict();
    void Test();
    // Test functions
    void TestBucket(std::string& pathDir, BucketConfig::TYPE_BUCKET_ID bucketID = 0);

    // For the first scheme:
    void InitBinaryTreeScheme1();
    void InitPathScheme1(PathConfig::TYPE_PATH_ID path_id);
    void InitSingleBucketScheme1(BucketConfig::TYPE_BUCKET_ID bucket_id);
    void InitEvictPathScheme1(PathConfig::TYPE_PATH_ID path_id);
    void ReadPathScheme1(BlockConfig::TYPE_BLOCK_ID block_id);
    // Log variables
    DurationLogger logger;
    std::string LogReadPathGenOffsets = "ReadPathGenOffsets";
    std::string LogReadPathDeserializeDecrypt = "ReadPathDeserializeDecrypt";
    std::string LogReadPathDeserialize = "ReadPathDeserialize";
    std::string LogReadPathDecrypt = "ReadPathDecrypt";
    std::string LogEvictProcessRootBucket = "EvictProcessRootBucket";
    std::string LogEvictGenTripletEvictRootPerms = "EvictGenTripletEvictRootPerms";
    std::string LogEvictGenTripletEvictPerms = "EvictGenTripletEvictPerms";

    std::string LogReadPathGenBIDsScheme1 = "ClientComputationReadPathGenBIDs";
    std::string LogReadPathUpdatePathIDScheme1 = "ClientComputationReadPathUpdatePathID";
    std::string LogReadPathGenOffsetsScheme1 = "ClientComputationReadPathGenOffsets";
    std::string LogReadPathSendOffsetsSchme1 = "ClientServerBandwidthReadPathSendOffsets";
    std::string LogReadPathSendPathMDsScheme1 = "ClientServerBandwidthReadPathSendPathMDs";
    std::string LogReadPathGenDummyXORScheme1 = "ClientComputationReadPathGenDummyXOR";
    std::string LogReadPathDecryptTargetBlockScheme1 = "ClientComputationReadPathDecryptTargetBlock";
    std::string LogReadPathStoreAccessedBlock2StashScheme1 = "ClientComputationReadPathStoreAccessedBlock2Stash";
    std::string LogReadPathSendAccessedBlocks2ThirdPartyScheme1 = "ClientBandwidthReadPathSendAccessedBlocks2ThirdParty";
    std::string LogReadPathEncryptBlockWithPathIdScheme1 = "ClientComputationReadPathEncryptBlockWithPathId";

    // For the second scheme:
    // 1. ReadPath operation
    std::string LogReadPathSendPathIDScheme2 = "ClientServerBandwidthReadPathSendPathID";
    std::string LogReadPathUpdatePathIDScheme2 = "ClientComputationReadPathUpdatePathID";
    std::string LogReadPathGenOffsetsScheme2 = "ClientComputationReadPathGenOffsets";
    std::string LogReadPathSendOffsetsScheme2 = "ClientServerBandwidthReadPathSendOffsets";
    std::string LogReadPathDecryptTargetBlockScheme2 = "ClientComputationReadPathDecryptTargetBlock";
    // 2. EarlyReshuffle operation
    std::string LogEarlyReshuffleGenPermsScheme2 = "ClientComputationEarlyReshuffleGenPerms";
    std::string LogEarlyReshuffleSendPermToThirdPartyScheme2 = "ClientThirdPartyBandwidthEarlyReshuffleSendPermToThirdParty";
    std::string LogEarlyReshuffleSendPermToServerScheme2 = "ClientServerBandwidthEarlyReshuffleSendPermToServer";
    // 3. Eviction operation
    std::string LogEvictProcessRootBucketScheme2 = "ClientComputationEvictProcessRootBucket";
    std::string LogEvictSendRootBucketToServerScheme2 = "ClientServerBandwidthEvictSendRootBucketToServer";
    std::string LogEvictGenTripletEvictPermsScheme2 = "ClientComputationEvictGenTripletEvictPerms";
    std::string LogEvictSendTripletEvictPermsToThirdPartyScheme2 = "ClientThirdPartyBandwidthEvictSendTripletEvictPermsToThirdParty";
    std::string LogEvictSendTripletEvictPermsToServerScheme2 = "ClientServerBandwidthEvictSendTripletEvictPermsToServer";
    std::string LogEvictGenBucketIDsScheme2 = "ClientComputationEvictGenBucketIDs";
    // Properties
    ClientConfig::TYPE_HOST host;
    ClientConfig::TYPE_PORT port; 
    SocketCommunicator communicator, communicator2ThirdParty;
    std::vector<BucketConfig::META_DATA> mds;
    // temporary variables
    std::vector<char> pathIDChars;
    ServerConfig::TYPE_CMD cmd;
    std::vector<char> blockIDChars;
    std::vector<BucketConfig::TYPE_SLOT_ID> offsets;
    std::vector<char> offsetsChars;
    ClientConfig::TYPE_CHAR_SIZE offsetsCharsSize;
    // std::vector<std::pair<ZZ, ZZ>> targetBlockCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> targetBlockCiphertexts;
    std::vector<char> targetBlockCiphertextsSerializedData;
    ElGamal_parallel_ntl elgamal;
    std::vector<char> targetBlockData;

    // Variable for the early reshuffle
    BucketConfig::META_DATA rootBucketMD;
    ClientConfig::TYPE_CHAR_SIZE permDataSize;
    std::vector<char> permData1;
    std::vector<char> permData2;
    std::vector<BucketConfig::TYPE_SLOT_ID> perm1, perm2;
    // Variables for the eviction
    // Eviciton initialization
    std::vector<BucketConfig::TYPE_BUCKET_ID> tripletBucketIDs;
    std::vector<BucketConfig::META_DATA> tripletBucketMDs;
    BucketConfig::TYPE_BUCKET_SIZE realBlockNumForEviction;
    // Eviction operation
    std::vector<char> rootBucketData;
    BucketConfig::TYPE_BUCKET_SIZE evictPermSize;
    std::vector<BucketConfig::TYPE_SLOT_ID> original_perms;
    std::vector<BucketConfig::TYPE_SLOT_ID> triplet_evict_perm1, triplet_evict_perm2;
    ClientConfig::TYPE_CHAR_SIZE triplet_evict_perm_size;
    std::vector<char> triplet_evict_permData1, triplet_evict_permData2;

    // Complete operations including createBinaryTree, readPath, earlyReshuffle, eviction
    void InitCreateBinaryTree();
    void ReadPathComplete(BlockConfig::TYPE_BLOCK_ID block_id);
    void EarlyReshuffleComplete(BucketConfig::TYPE_BUCKET_ID bucket_id);
    void EvictComplete(PathConfig::TYPE_PATH_ID path_id);
    void GenTripletEvictPerms(std::vector<BucketConfig::META_DATA>& tripletBucketMDS,
                              std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBucketIDs,
                              PathConfig::TYPE_PATH_SIZE height,
                              PathConfig::TYPE_PATH_SIZE curLevel,
                              std::vector<BucketConfig::TYPE_SLOT_ID>& tripletPermCur,
                              std::vector<BucketConfig::TYPE_SLOT_ID>& tripletPermTarget);

    // Variables
    // std::vector<BucketConfig::META_DATA> treeMetaDatas;
    std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA> treeMetaDatas;
    Tree tree;
    std::vector<BlockConfig::TYPE_BLOCK_ID> PositionMap;
    PathConfig::TYPE_PATH_ID pathIDComplete;
    std::vector<BucketConfig::TYPE_BUCKET_ID> bucketIDOffsets;
    Path pathComplete;
    int counter = 0;
    std::vector<BucketConfig::TYPE_SLOT_ID> originalPermComplete;
    std::vector<BucketConfig::TYPE_SLOT_ID> perm1Complete, perm2Complete;
      // Variables for the readPath
    // std::vector<bool> isInStash;
    std::unordered_map<BlockConfig::TYPE_BLOCK_ID, std::pair<std::vector<std::pair<ZZ_p, ZZ_p>>, std::vector<char>>> blockDataStash;

    // Variables for the eviction complete
    std::vector<char> rootBucketDataEvictComplete;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> rootBucketCiphertextsEvictComplete;
    std::vector<char> rootBlockCiphertextsSerializedEvictComplete;
    // std::vector<std::pair<ZZ_p, ZZ_p>> rootBlockCiphertextsEvictComplete;
    BucketConfig::TYPE_BUCKET_SIZE counterRealBlockToRootBucket = 0;
    std::vector<BlockConfig::TYPE_BLOCK_ID> blockIDsDeletedFromStash;
    const BucketConfig::TYPE_BUCKET_ID rootBucketIDComplete = 0;
    std::vector<BucketConfig::TYPE_SLOT_ID> tripletPermCurComplete, tripletPermTargetComplete, tripletPermIntermediateComplete;
    std::vector<BucketConfig::TYPE_SLOT_ID> tripletPermIntermediateCompleteThirdParty, tripletPermIntermediateCompleteServer, tripletPermIntermediateCompleteBoth;
    int tripletPermIntermediateCompleteBothSize;
    std::vector<BucketConfig::META_DATA> tripletBucketMDsComplete;
    std::vector<BucketConfig::TYPE_BUCKET_ID> tripletBucketIDsComplete;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketIDsCompleteCharNum;
    std::vector<BucketConfig::TYPE_BUCKET_ID> evictPathBucketIDsComplete;
    std::vector<std::pair<BucketConfig::TYPE_SLOT_ID, BucketConfig::TYPE_BUCKET_ID>> swapIndexes;


    // Scheme 1
    AES_CTR aes_client;
    unsigned char iv_client[AES_BLOCK_SIZE];
    // PathConfig::TYPE_PATH_ID pathIDScheme1;
    std::vector<char> pathBucketMDsSerializedData1;
    ClientConfig::TYPE_CHAR_SIZE pathBucketMDsSerializedDataSize1;
    BucketConfig::META_DATA bucketMD1;
    std::vector<char> targetBlockCipherData1;
    std::vector<char> targetBlockData1;
    bool findTargetBlock1 = false;
    std::vector<char> blocksInStashData1;
    SizeConfig::TYPE_UNSIGNED_SIZE blocksInStashDataSize1;
    SizeConfig::TYPE_UNSIGNED_SIZE blockInStashDataSize1;
    // BucketConfig::TYPE_SLOT_ID targetBucketSlotID1;
    // PathConfig::TYPE_PATH_SIZE targetBucketID1;
    BucketConfig::TYPE_SLOT_ID targetBlockIndex1;
    std::vector<char> recoverDummyBlockData1;
    std::vector<char> recoverDummyBlockCipherData1;
    Block block1;
    std::vector<BucketConfig::TYPE_BUCKET_ID> ivIndex1;
    std::unordered_map<BlockConfig::TYPE_BLOCK_ID, std::pair<PathConfig::TYPE_PATH_ID, std::vector<char>>> blockDataStash1;
    SizeConfig::TYPE_UNSIGNED_SIZE blockStashSize1;

    // functions used to test the scheme 1
    void EarlyReshuffleScheme1(BucketConfig::TYPE_BUCKET_ID bucket_id);
    void EvictScheme1(PathConfig::TYPE_PATH_ID path_id);
    // Open SSL variables
    std::vector<std::vector<BIGNUM*>> bucketCiphertextsBNSgx;
    int bucketCiphertextsBNSgxSize;
};
#endif // CLIENT_H