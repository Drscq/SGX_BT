import sys
import os

# Add the parent directory to the system path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from config import dir_layer_1, dir_layer_2, dir_layer_3, dir_layer_4, dir_layer_5

def create_directories(base_path, sub_dirs):
    """
    Create directories based on the provided base path and subdirectories.

    Args:
        base_path (str): The base path where directories will be created.
        sub_dirs (list): A list of subdirectory names to create.
    """
    for sub_dir in sub_dirs:
        dir_path = os.path.join(base_path, sub_dir)
        os.makedirs(dir_path, exist_ok=True)
        print(f"Created directory: {dir_path}")

def main():
    """
    Main function to create organized directories.
    """
    # Define the base path for the directories
    base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../CSH_SGX_STAR'))
    # Create directories for each layer
    if not os.path.exists(base_path):
        create_directories(base_path, dir_layer_1)
    for layer_1 in dir_layer_1:
        layer_1_path = os.path.join(base_path, layer_1)
        if not os.path.exists(layer_1_path):
            create_directories(layer_1_path, dir_layer_2)
        for layer_2 in dir_layer_2:
            layer_2_path = os.path.join(layer_1_path, layer_2)
            if not os.path.exists(layer_2_path):
                create_directories(layer_2_path, dir_layer_3)
            for layer_3 in dir_layer_3:
                layer_3_path = os.path.join(layer_2_path, layer_3)
                if not os.path.exists(layer_3_path):
                    create_directories(layer_3_path, dir_layer_4)
                for layer_4 in dir_layer_4:
                    layer_4_path = os.path.join(layer_3_path, layer_4)
                    if not os.path.exists(layer_4_path):
                        create_directories(layer_4_path, dir_layer_5)
                    for layer_5 in dir_layer_5:
                        layer_5_path = os.path.join(layer_4_path, layer_5)
                        os.makedirs(layer_5_path, exist_ok=True)
                        # # clean up all in the current directory
                        # os.system(f"rm -rf {layer_5_path}/*")
    
    base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../CSH_SGX'))
    # Create directories for each layer
    if not os.path.exists(base_path):
        create_directories(base_path, dir_layer_1)
    for layer_1 in dir_layer_1:
        layer_1_path = os.path.join(base_path, layer_1)
        if not os.path.exists(layer_1_path):
            create_directories(layer_1_path, dir_layer_2)
        for layer_2 in dir_layer_2:
            layer_2_path = os.path.join(layer_1_path, layer_2)
            if not os.path.exists(layer_2_path):
                create_directories(layer_2_path, dir_layer_3)
            for layer_3 in dir_layer_3:
                layer_3_path = os.path.join(layer_2_path, layer_3)
                if not os.path.exists(layer_3_path):
                    create_directories(layer_3_path, dir_layer_4)
                for layer_4 in dir_layer_4:
                    layer_4_path = os.path.join(layer_3_path, layer_4)
                    if not os.path.exists(layer_4_path):
                        create_directories(layer_4_path, dir_layer_5)
                    for layer_5 in dir_layer_5:
                        layer_5_path = os.path.join(layer_4_path, layer_5)
                        os.makedirs(layer_5_path, exist_ok=True)
                        # # clean up all in the current directory
                        # os.system(f"rm -rf {layer_5_path}/*")

if __name__ == "__main__":
    main()