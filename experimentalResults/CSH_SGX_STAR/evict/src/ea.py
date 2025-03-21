categories = {
    "ClientComputation": r"ClientComputation.*?:\s*(\d+)\s+ns",
    "ServerComputation": r"ServerComputation.*?:\s*(\d+)\s+ns",
    "ThirdPartyComputation": r"HelperComputation.*?:\s*(\d+)\s+ns",
    "ServerIO": r"DiskIO.*?:\s*(\d+)\s+ns",
    "ClientServerBandwidth": r"ClientCommunication.*?:\s*(\d+)\s+ns",
    "ServerClientBandwidth": r"ServerCommunication.*?:\s*(\d+)\s+ns"
}

def get_height_val(file_name):
    match = re.search(r'Height_(\d+)', file_name)
    return int(match.group(1)) if match else 0


import os
import re
logs_dir = "../logs"
files = os.listdir(logs_dir)
files_sorted = sorted(files, key=get_height_val)

def extract_and_sum_values(content):
    sums = {cat: 0 for cat in categories}
    for cat, pat in categories.items():
        matches = re.findall(pat, content)
        for m in matches:
            sums[cat] += int(m)
    return sums

def get_all_categories_total(content):
    sums = extract_and_sum_values(content)
    return sum(sums.values())
evict_total_delays = {}
access_time = 3
for file in files_sorted:
    height = get_height_val(file)
    with open(os.path.join(logs_dir, file), 'r') as f:
        content = f.read()
        results = extract_and_sum_values(content)
        scb_val = results["ServerClientBandwidth"]
        for category, total in results.items():
            if category == "ClientServerBandwidth":
                total = total + scb_val
            # if category == "ServerClientBandwidth": # remove this category from the results
            if category == "ServerClientBandwidth":
                total = 0
            

            # print(f"{category}: {total} ns")
        total = get_all_categories_total(content)
        evict_total_delays[height] = total // access_time

# # for height, total in evict_total_delays.items():
# #     print(f"Height: {height}, Total Delay: {total} ns")

# sort the dictionary by its values
sorted_dict = dict(sorted(evict_total_delays.items(), key=lambda item: item[1]))
for height, total in sorted_dict.items():
    print(f"Height: {height}, Total Delay: {total} ns")


