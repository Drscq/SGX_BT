import os
import sys
import subprocess
import math
from tools.updateConfig import update_config
from config import dir_layer_1, dir_layer_2, dir_layer_3, dir_layer_4, dir_layer_5
config_file = '../../SampleEnclave/App/src/config.h'
config_sgx_file = '../../SampleEnclave/App/src/configSgx.h'
host = '127.0.0.1'
TNRBs = [18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
A = 43
build_dir_server = '../../SampleEnclave'
build_cmd_server = f"cd {build_dir_server} && make clean && make -j 40"
bin_dir_server_base_css = '../CSH_SGX_STAR'
bin_dir_server_basr_cs = '../CSH_SGX'
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
                        print(f"Created directory: {bin_dir_path}")
    return dir_paths
def compile_move_breakdowncost_64KB():
    block_size = 64 * 1024
    dir_layer_3_ = ['64KB']
    dir_layer_4_client = ['client']
    dir_layer_4_server = ['server']
    bin_dir_paths_server = create_bin_dir(bin_dir_server_base_css, dir_layer_1, dir_layer_2, dir_layer_3_, dir_layer_4_server, dir_layer_5)
    bin_dir_paths_client = create_bin_dir(bin_dir_server_basr_cs, dir_layer_1, dir_layer_2, dir_layer_3_, dir_layer_4_client, dir_layer_5)
    for tnrb in TNRBs:
        height = math.ceil(math.log2(math.ceil(2**tnrb / A))) + 2
        update_config(config_file, h=host,height=height, bs=block_size)
        update_config(config_sgx_file, h=host,height=height, bs=block_size)
        subprocess.run(build_cmd_server, shell=True)
        for i, bin_dir_path in enumerate(bin_dir_paths_server):
            bin_dir_path = os.path.join(bin_dir_path, f"tnrb_{tnrb}")
            os.makedirs(bin_dir_path, exist_ok=True)

            subprocess.run(f"cp {build_dir_server}/app {bin_dir_path}/csh_sgx_star_{tnrb}", shell=True)

compile_move_breakdowncost_64KB()


    