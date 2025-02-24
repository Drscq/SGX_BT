config_dir = "../../../../../SampleEnclave/App/src"
import os
config_path = os.path.join(config_dir, "config.h")
configSgx_path = os.path.join(config_dir, "configSgx.h")

import re
import math
port = 2000
host = "198.82.162.120"

def update_config(config_path, height, port, blockSize, host):
    """
    Function to update configuration values in the config.h file

    Args:
    config_path: str, path to the config.h file
    height: int, height value
    port: int, port value
    blockSize: int, blockSize value
    host: str, host value

    Returns:
    None
    """
    with open(config_path, "r") as f:
        lines = f.readlines()
    # Interate though each line and apply the changes 
    for i, line in enumerate(lines):
        # update macro value of USE_PRINT_TARGET_BLOCK
        if re.match(r'#define\s+\w+\s+\d+', line):
            lines[i] = re.sub(r'(\s+\d+)$', ' 0', line)
    for i, line in enumerate(lines):
        if "#define LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT " in line:
            lines[i] = re.sub(r'\s+\d+', ' 1', line)
        if "#define LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER " in line:
            lines[i] = re.sub(r'\s+\d+', ' 1', line)
        # update HEIGHT value
        if "inline constexpr TYPE_PATH_SIZE HEIGHT =" in line:
            lines[i] = re.sub(r'\d+', str(height), line)
        if "    inline constexpr TYPE_PATH_SIZE_SGX HEIGHT = " in line:
            lines[i] = re.sub(r'\d+', str(height), line)
        # update PORT value in server
        if "inline int PORT = " in line:
            lines[i] = re.sub(r'\d+', str(port), line)
        # update PORT value in client
        if "inline TYPE_PORT PORT =" in line:
            lines[i] = re.sub(r'\d+', str(port), line)
        # update BLOCK_SIZE value
        if "inline TYPE_BLOCK_SIZE BLOCK_SIZE =" in line:
            lines[i] = re.sub(r'\d+', str(blockSize), line)
            # print(f"Updated BLOCK_SIZE: {lines[i]}")
        # update BLOCK_SIZE_SGX value in configSgx.h
        if "inline TYPE_BLOCK_SIZE_SGX BLOCK_SIZE_SGX = " in line:
            lines[i] = re.sub(r'\d+', str(blockSize), line)
        # update HOST value
        if "inline TYPE_HOST HOST =" in line:
            lines[i] = re.sub(r'\".*\"', f'"{host}"', line)
    # Write the changes back to the file
    with open(config_path, "w") as f:
        f.writelines(lines)

# # Example usage
# update_config(config_path, height, port, blockSize, host)

BS = 64 * 1024
# TNRBs = [18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
TNRBs = [18]
A = 43
build_dir = "../../../../../SampleEnclave"
build_cmd = f"cd {build_dir} && make clean && make"
src_file = f"{build_dir}/app"
bin_dir = "../../../../../SampleEnclave/bin/earlyReshuffle1"
def main():
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)
    else:
        os.system(f"rm -rf {bin_dir}/*")
    
    import subprocess
    for TNRB in TNRBs:
        height = math.ceil(math.log2(math.ceil(2**TNRB / A))) + 2
        print(f"Updating config file for TNRB = {TNRB}")
        update_config(config_path, height, port, BS, host)
        update_config(configSgx_path, height, port, BS, host)
        dest_file = f"{bin_dir}/CSH_TNRB_{TNRB}"
        mv_cmd = f"cp {src_file} {dest_file}"
        subprocess.Popen(build_cmd, shell=True).wait()
        subprocess.Popen(mv_cmd, shell=True).wait()
        print(f"Build for TNRB = {TNRB} finished")

if __name__ == "__main__":
    main()

    