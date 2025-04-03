# Global Variables
dir_layer_1 = ['readPath', "earlyReshuffle", "evictPath"]
dir_layer_2 = ['breakDownCost']
dir_layer_3 = ['64KB', "256KB", "1024KB", 'blockSize']


def clean_directory_with_exception(
    directory: str,
    exception: str,
    ignore_errors: bool = False,
) -> None:
    """
    Cleans a directory by removing all files and subdirectories, except for the specified exception.

    Args:
        directory (str): The path to the directory to clean.
        exception (str): The name of the file or subdirectory to exclude from deletion.
        ignore_errors (bool): If True, ignores errors during deletion. Defaults to False.

    Raises:
        ValueError: If the specified directory does not exist or is not a directory.
    """
    import os
    import shutil

    if not os.path.exists(directory):
        raise ValueError(f"The specified directory does not exist: {directory}")
    
    if not os.path.isdir(directory):
        raise ValueError(f"The specified path is not a directory: {directory}")

    exception_path = os.path.join(directory, exception)

    for root, dirs, files in os.walk(directory, topdown=False):
        for name in files:
            file_path = os.path.join(root, name)
            if file_path != exception_path and not file_path.startswith(exception_path + os.sep):
                try:
                    os.remove(file_path)
                except Exception as e:
                    if not ignore_errors:
                        raise e
        for name in dirs:
            dir_path = os.path.join(root, name)
            if dir_path != exception_path and not dir_path.startswith(exception_path + os.sep):
                try:
                    shutil.rmtree(dir_path)
                except Exception as e:
                    if not ignore_errors:
                        raise e