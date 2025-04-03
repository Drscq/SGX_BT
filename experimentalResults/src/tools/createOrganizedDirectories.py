import sys
import os

# Add the parent directory to the system path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from config import dir_layer_1, dir_layer_2, dir_layer_3

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

if __name__ == "__main__":
    main()