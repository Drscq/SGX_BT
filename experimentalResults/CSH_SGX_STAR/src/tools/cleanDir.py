import sys
import os

# Add the parent directory to the system path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from config import clean_directory_with_exception

dir_clean = "../../"
print(os.listdir(dir_clean))
clean_directory_with_exception(dir_clean, "src")
print(os.listdir(dir_clean))