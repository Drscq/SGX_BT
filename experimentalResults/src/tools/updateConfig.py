import re
host = "198.82.162.120"
real_block_capacity = 30
dummy_block_capacity = 43
eviction_frequency = dummy_block_capacity
port = 8183
threads_num_server = 32
helper_threads_num = 4
def update_config(config_file,
                   h=host,
                   rbc=real_block_capacity,
                   dbc=dummy_block_capacity,
                   p=port,
                   tns=threads_num_server,
                   htn=helper_threads_num,
                   height=3,
                   bs=256,
                   ef=eviction_frequency):
    with open(config_file, 'r') as file:
        lines = file.readlines()
    for i, line in enumerate(lines):
        if re.match(r'#define\s+\w+\s+\d+', line):
            lines[i] = re.sub(r'(\s+\d+)$', ' 0', line)
    for i, line in enumerate(lines):
        if '#define MULTI_THREAD_RERANDOMIZE_SWITCH ' in line:
            lines[i] = re.sub(r'\s+\d+', ' 1', line)
        if "inline ServerConfig::TYPE_PORT_NUM PORT = " in line:
            lines[i] = re.sub(r'\d+', str(p), line) 
        if "inline constexpr TYPE_PATH_SIZE HEIGHT = " in line:
            lines[i] = re.sub(r'\d+', str(height), line)
        if "inline TYPE_BLOCK_SIZE BLOCK_SIZE = " in line:
            lines[i] = re.sub(r'\d+', str(bs), line)
        if "inline TYPE_HOST HOST = " in line:
            lines[i] = re.sub(r'\".*\"', f'"{h}"', line)
        if "inline TYPE_PORT PORT = " in line:
            lines[i] = re.sub(r'\d+', str(p), line)
        if "const TYPE_BUCKET_SIZE BUCKET_REAL_BLOCK_CAPACITY = " in line:
            lines[i] = re.sub(r'\d+', str(rbc), line)
        if "const TYPE_BUCKET_SIZE BUCKET_DUMMY_BLOCK_CAPACITY = " in line:
            lines[i] = re.sub(r'\d+', str(dbc), line)
        if "inline BucketConfig::TYPE_BUCKET_SIZE EVICTION_FREQUENCY = " in line:
            lines[i] = re.sub(r'\d+', str(ef), line)
        if "#define USE_OPENSSL" in line:
            lines[i] = re.sub(r'\s+\d+', ' 1', line)
        if "const TYPE_UNSIGNED_SIZE_SGX BUCKET_REAL_BLOCK_CAPACITY_SGX = " in line:
            lines[i] = re.sub(r'\d+', str(rbc), line)
        if "const TYPE_UNSIGNED_SIZE_SGX BUCKET_DUMMY_BLOCK_CAPACITY_SGX = " in line:
            lines[i] = re.sub(r'\d+', str(dbc), line)
        if "inline TYPE_BLOCK_SIZE_SGX BLOCK_SIZE_SGX = " in line:
            lines[i] = re.sub(r'\d+', str(bs), line)
        if "inline constexpr TYPE_PATH_SIZE_SGX HEIGHT = " in line:
            lines[i] = re.sub(r'\d+', str(height), line)
    
    with open(config_file, 'w') as file:
        file.writelines(lines)


# SampleEnclave/App/src/config.h
config_file = '../../../SampleEnclave/App/src/config.h'
config_sgx_file = '../../../SampleEnclave/App/src/configSgx.h'
def main():
    update_config(config_file)
    update_config(config_sgx_file)

if __name__ == "__main__":
    main()
    