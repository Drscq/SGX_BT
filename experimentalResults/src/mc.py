import os
import sys
import subprocess
import math
from tools.updateConfig import update_config
from config import dir_layer_1, dir_layer_2, dir_layer_3, dir_layer_4, dir_layer_5
config_file = '../../SampleEnclave/App/src/config.h'
config_sgx_file = '../../SampleEnclave/App/src/configSgx.h'
host = '128.173.236.241'
TNRBs = [18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
A = 43
build_dir_server = '../../SampleEnclave'
build_cmd_server = f"cd {build_dir_server} && make clean && make -j 40"
bin_dir_server_base_css = '../CSH_SGX_STAR'
bin_dir_server_base_cs = '../CSH_SGX'
build_dir_client = '../../SampleEnclave/App/src/build'
build_cmd_client = f"cd {build_dir_client} && make crtgamal_client -j 10"
def create_bin_dir(base_dir, dir_layer_1, dir_layer_2, dir_layer_3, dir_layer_4, dir_layer_5):
    """
    Create directories based on the provided base path and subdirectories.
    """
    dir_paths = []
    for layer_1 in dir_layer_1:
        layer_1_path = os.path.join(base_dir, layer_1)
        for layer_2 in dir_layer_2:
            layer_2_path = os.path.join(layer_1_path, layer_2)
            for layer_3 in dir_layer_3:
                layer_3_path = os.path.join(layer_2_path, layer_3)
                for layer_4 in dir_layer_4:
                    layer_4_path = os.path.join(layer_3_path, layer_4)
                    for layer_5 in dir_layer_5:
                        bin_dir_path = os.path.join(layer_4_path, layer_5)
                        dir_paths.append(bin_dir_path)
    return dir_paths
def compile_move_breakdowncost_64KB(target):
    """
    Compile and move binaries for server or client based on the target input.
    :param target: 'server' or 'client'
    """
    block_size = 64 * 1024
    dir_layer_3_ = ['64KB']
    dir_layer_4_client = ['client']
    dir_layer_4_server = ['server']

    if target == 'server':
        bin_dir_paths = create_bin_dir(bin_dir_server_base_css, dir_layer_1, dir_layer_2, dir_layer_3_, dir_layer_4_server, dir_layer_5)
        print("bin_dir_paths_server:")
    elif target == 'client':
        bin_dir_paths = create_bin_dir(bin_dir_server_base_css, dir_layer_1, dir_layer_2, dir_layer_3_, dir_layer_4_client, dir_layer_5)
        print("bin_dir_paths_client:")
    else:
        raise ValueError("Invalid target. Use 'server' or 'client'.")

    for bin_dir_path in bin_dir_paths:
        print(bin_dir_path)

    for tnrb in TNRBs:
        height = math.ceil(math.log2(math.ceil(2**tnrb / A))) + 2
        update_config(config_file, h=host, height=height, bs=block_size)
        update_config(config_sgx_file, h=host, height=height, bs=block_size)

        if target == 'server':
            subprocess.Popen(build_cmd_server, shell=True).wait()
            for bin_dir_path in bin_dir_paths:
                cp_cmd_server = f"cp {build_dir_server}/app {bin_dir_path}/csh_sgx_star_{tnrb}"
                print(f"Copying server binary to {bin_dir_path}")
                subprocess.Popen(cp_cmd_server, shell=True).wait()
        elif target == 'client':
            subprocess.Popen(build_cmd_client, shell=True).wait()
            for bin_dir_path in bin_dir_paths:
                cp_cmd_client = f"cp {build_dir_client}/crtgamal_client {bin_dir_path}/csh_sgx_{tnrb}"
                print(f"Copying client binary to {bin_dir_path}")
                subprocess.Popen(cp_cmd_client, shell=True).wait()

# use the main function to take the system arguments as the inputs to indicate the target
def main():
    if len(sys.argv) != 2:
        print("Usage: python mc.py <target>")
        print("target: server or client")
        sys.exit(1)
    target = sys.argv[1]
    if target not in ['server', 'client']:
        print("Invalid target. Use 'server' or 'client'.")
        sys.exit(1)

    compile_move_breakdowncost_64KB(target)

if __name__ == "__main__":
    main()


    