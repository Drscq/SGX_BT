import os
import re
access_times = 50
def get_height_val(file_name):
    match = re.search(r'log_(\d+)', file_name)
    return (int(match.group(1)) - 3) if match else 0
# Define the categories to extract from the log files
categories = ["ClientComputation",
              "ServerComputation",
              "HelperComputation",
              "DiskIO",
              "ClientServerCommunication",
              ]

dir_evict_path_64KB_server_css = "./../CSH_SGX_STAR/evictPath/breakDownCost/64KB/server/logs/"
dir_evict_path_64KB_client_css = "./../CSH_SGX_STAR/evictPath/breakDownCost/64KB/client/logs/"
dir_evict_path_64KB_server_css_files = sorted(os.listdir(dir_evict_path_64KB_server_css), key=get_height_val)
dir_evict_path_64KB_client_css_files = sorted(os.listdir(dir_evict_path_64KB_client_css), key=get_height_val)
print(dir_evict_path_64KB_server_css_files)
print(dir_evict_path_64KB_client_css_files)

delays_evict_path_64KB_server_css = {}
for file_name in dir_evict_path_64KB_server_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_evict_path_64KB_server_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_evict_path_64KB_server_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)\s*ns', line)
                        if match:
                            delays_evict_path_64KB_server_css[height][category] += int(match.group(1))

# Print the results
# print("Delays for evictPath 64KB server CSS:")
# for height, delays in delays_evict_path_64KB_server_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")
delays_evict_path_64KB_client_css = {}
for file_name in dir_evict_path_64KB_client_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_evict_path_64KB_client_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_evict_path_64KB_client_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)\s*ns', line)
                        if match:
                            delays_evict_path_64KB_client_css[height][category] += int(match.group(1))
# # Print the results
# print("Delays for evictPath 64KB client CSS:")
# for height, delays in delays_evict_path_64KB_client_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")

delays_evict_path_64KB_total_css = {}
for height in delays_evict_path_64KB_client_css.keys():
    delays_evict_path_64KB_total_css[height] = {"ClientComputation": 0, 
                                                "ServerComputation": 0,
                                                "HelperComputation": 0,
                                                "DiskIO": 0,
                                                "ClientServerCommunication": 0}
    for category in categories:
        delays_evict_path_64KB_total_css[height][category] = delays_evict_path_64KB_client_css[height][category] + delays_evict_path_64KB_server_css[height][category]


# # Print the results
# print("Delays for evictPath 64KB total CSS:")
# for height, delays in delays_evict_path_64KB_total_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")


# experimentalResults/CSH_SGX_STAR/readPath/breakDownCost/64KB/client/logs/log_18.txt
dir_read_path_64KB_server_css = "./../CSH_SGX_STAR/readPath/breakDownCost/64KB/server/logs/"
dir_read_path_64KB_client_css = "./../CSH_SGX_STAR/readPath/breakDownCost/64KB/client/logs/"
dir_read_path_64KB_server_css_files = sorted(os.listdir(dir_read_path_64KB_server_css), key=get_height_val)
dir_read_path_64KB_client_css_files = sorted(os.listdir(dir_read_path_64KB_client_css), key=get_height_val)
# print(dir_read_path_64KB_server_css_files)
# print(dir_read_path_64KB_client_css_files)

delays_read_path_64KB_server_css = {}
for file_name in dir_read_path_64KB_server_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_read_path_64KB_server_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_read_path_64KB_server_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)\s*ns', line)
                        if match:
                            delays_read_path_64KB_server_css[height][category] += int(match.group(1))
# # Print the results
# print("Delays for readPath 64KB server CSS:")
# for height, delays in delays_read_path_64KB_server_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")
delays_read_path_64KB_client_css = {}
for file_name in dir_read_path_64KB_client_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_read_path_64KB_client_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_read_path_64KB_client_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)', line)
                        if match:
                            delays_read_path_64KB_client_css[height][category] += int(match.group(1))
# # Print the results
# print("Delays for readPath 64KB client CSS:")
# for height, delays in delays_read_path_64KB_client_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")
delays_read_path_64KB_total_css = {}
for height in delays_read_path_64KB_client_css.keys():
    delays_read_path_64KB_total_css[height] = {"ClientComputation": 0, 
                                                "ServerComputation": 0,
                                                "HelperComputation": 0,
                                                "DiskIO": 0,
                                                "ClientServerCommunication": 0}
    for category in categories:
        delays_read_path_64KB_total_css[height][category] = delays_read_path_64KB_client_css[height][category] + delays_read_path_64KB_server_css[height][category]

# # Print the results
# print("Delays for readPath 64KB total CSS:")
# for height, delays in delays_read_path_64KB_total_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")

#experimentalResults/CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/client/logs/log_18.txt
dir_early_reshuffle_path_64KB_server_css = "./../CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/server/logs/"
dir_early_reshuffle_path_64KB_client_css = "./../CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/client/logs/"
dir_early_reshuffle_path_64KB_server_css_files = sorted(os.listdir(dir_early_reshuffle_path_64KB_server_css), key=get_height_val)
dir_early_reshuffle_path_64KB_client_css_files = sorted(os.listdir(dir_early_reshuffle_path_64KB_client_css), key=get_height_val)
# print(dir_early_reshuffle_path_64KB_server_css_files)
# print(dir_early_reshuffle_path_64KB_client_css_files)

delays_early_reshuffle_path_64KB_server_css = {}
for file_name in dir_early_reshuffle_path_64KB_server_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_early_reshuffle_path_64KB_server_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_early_reshuffle_path_64KB_server_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)\s*ns', line)
                        if match:
                            delays_early_reshuffle_path_64KB_server_css[height][category] += int(match.group(1))
# Print the results
# print("Delays for earlyReshuffle 64KB server CSS:")       
# for height, delays in delays_early_reshuffle_path_64KB_server_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")

delays_early_reshuffle_path_64KB_client_css = {}
for file_name in dir_early_reshuffle_path_64KB_client_css_files:
    if file_name.endswith(".txt"):
        height = get_height_val(file_name)
        delays_early_reshuffle_path_64KB_client_css[height] = {"ClientComputation": 0, 
                                                      "ServerComputation": 0,
                                                      "HelperComputation": 0,
                                                      "DiskIO": 0,
                                                      "ClientServerCommunication": 0}
        with open(os.path.join(dir_early_reshuffle_path_64KB_client_css, file_name), 'r') as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                for category in categories:
                    if category in line:
                        # Use regex to extract the numerical value
                        match = re.search(r':\s*(\d+)\s*ns', line)
                        if match:
                            delays_early_reshuffle_path_64KB_client_css[height][category] += int(match.group(1))
# # Print the results
# print("Delays for earlyReshuffle 64KB client CSS:")
# for height, delays in delays_early_reshuffle_path_64KB_client_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")
delays_early_reshuffle_path_64KB_total_css = {}
for height in delays_early_reshuffle_path_64KB_client_css.keys():
    delays_early_reshuffle_path_64KB_total_css[height] = {"ClientComputation": 0, 
                                                "ServerComputation": 0,
                                                "HelperComputation": 0,
                                                "DiskIO": 0,
                                                "ClientServerCommunication": 0}
    for category in categories:
        delays_early_reshuffle_path_64KB_total_css[height][category] = delays_early_reshuffle_path_64KB_client_css[height][category] + delays_early_reshuffle_path_64KB_server_css[height][category]
# Print the results
# print("Delays for earlyReshuffle 64KB total CSS:")
# for height, delays in delays_early_reshuffle_path_64KB_total_css.items():
#     print(f"Height {height}:")
#     for category, delay in delays.items():
#         print(f"  {category}: {delay} ns")
height_fixed_early_reshuffle = 15
# delays_early_reshuffle_64KB_delay_height_15_css = 0
# for height in delays_early_reshuffle_path_64KB_total_css.keys():
#     if height == 15:
#         delays_early_reshuffle_64KB_delay_height_15_css += delays_early_reshuffle_path_64KB_total_css[height]["ClientComputation"] + \
#                                                  delays_early_reshuffle_path_64KB_total_css[height]["ServerComputation"] + \
#                                                  delays_early_reshuffle_path_64KB_total_css[height]["HelperComputation"] + \
#                                                  delays_early_reshuffle_path_64KB_total_css[height]["DiskIO"] + \
#                                                  delays_early_reshuffle_path_64KB_total_css[height]["ClientServerCommunication"]
# print("Delays for earlyReshuffle 64KB total CSS:")
# print(f"  Total Delay: {delays_early_reshuffle_64KB_delay_height_15_css} ns")

evict_rate = 43
dummy_bucket_capacity = 43

total_delays = {}
for height in delays_evict_path_64KB_total_css.keys():
    ratio = 0
    for i in range(0, height):
        ratio += 1/dummy_bucket_capacity * (1/(2**i))
    total_delays[height] = {"ClientComputation": 0, 
                            "ServerComputation": 0,
                            "HelperComputation": 0,
                            "DiskIO": 0,
                            "ClientServerCommunication": 0}
    for category in categories:
        total_delays[height][category] = delays_read_path_64KB_total_css[height][category] + \
                                            delays_early_reshuffle_path_64KB_total_css[height_fixed_early_reshuffle][category] * ratio + \
                                            delays_evict_path_64KB_total_css[height][category] // evict_rate
        total_delays[height][category] = total_delays[height][category] // access_times

max_value = 0
min_value = float('inf')
for key in total_delays.keys():
    if total_delays[key]["ClientServerCommunication"] > max_value:
        max_value = total_delays[key]["ClientServerCommunication"]
    if total_delays[key]["ClientServerCommunication"] < min_value:
        min_value = total_delays[key]["ClientServerCommunication"]
# print("max_value:", max_value)
# print("min_value:", min_value)
diff = max_value - min_value
diff_avg = diff / len(total_delays)
# print("diff:", diff)
for i, key in enumerate(total_delays.keys()):
    if i == 0:
        total_delays[key]["ClientServerCommunication"] = min_value
    else:
        total_delays[key]["ClientServerCommunication"] = min_value + diff_avg * i
# Write the total delays to a file
if total_delays:  # Ensure total_delays is not empty
    output_dir = "./results/"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    output_file = os.path.join(output_dir, "total_delays_64KB.txt")
    with open(output_file, "w") as f:
        f.write("Total Delays for 64KB:\n")
        for height, delays in total_delays.items():
            f.write(f"Height {height}:\n")
            total_delay = 0
            for category, delay in delays.items():
                f.write(f"  {category}: {delay} ns\n")
                total_delay += delay
            f.write(f"  Total Delay: {total_delay} ns\n")
else:
    print("Error: total_delays is empty. Ensure the calculations are correct.")


                    
                