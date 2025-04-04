import paramiko
import time
import subprocess
import os

# SSH constants
SSH_HOST = "asap.cs.vt.edu"
SSH_USER = input("Enter your username: ")
SSH_PASS = input("Enter your password: ")
p_server = 8183
def manage_ssh_connection(host, user, password):
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh_client.connect(host, username=user, password=password)
    return ssh_client

def stop_remote_process(ssh_client, port_number):
    cmd_find_process = f"lsof -t -i :{port_number}"
    stdin, stdout, stderr = ssh_client.exec_command(cmd_find_process)
    pid = stdout.read().decode().strip()
    if pid:
        cmd_kill_process = f"kill -9 {pid}"
        ssh_client.exec_command(cmd_kill_process)
        print(f"Killed process with PID {pid} on port {port_number}.")
    else:
        print(f"No process found on port {port_number}.")
    time.sleep(2)

def start_remote_process(ssh_client, command, log_file, working_directory):
    full_command = f"cd {working_directory} && nohup {command} >> {log_file} 2>&1 &"
    ssh_transport = ssh_client.get_transport()
    channel = ssh_transport.open_session()
    channel.exec_command(full_command)
    exit_status = channel.recv_exit_status()
    if exit_status == 0:
        print(f"Process started successfully: {command}")
    else:
        print(f"Failed to start process: {command}, Exit Status: {exit_status}")
    time.sleep(1)  # Give it some time to fully start

def check_process(ssh, port):
    stdin, stdout, stderr = ssh.exec_command(f'lsof -i:{port}')
    output = stdout.read().decode()
    if output:
        return True
    return False

def run_server(ssh_client, index, working_dir, cmd_run_server, server_log_file):
    # working_dir = "/home/changqi/ORAM/firstScheme/TriConverge-ORAM/Experiments/TriConvergeStar/eviction/BreakdownCost/256KB/bin"
    # cmd_run_server = f"./TriConvergeStar_TNRB_{index} server"
    # server_log_file = f"/tmp/server_{index}.log"

    print(f"Running server for index = {index} on the port {p_server}.")
    start_remote_process(ssh_client, cmd_run_server, server_log_file, working_dir)
    while not check_process(ssh_client, p_server):
        time.sleep(1)
    if check_process(ssh_client, p_server):
        print(f"Server for index = {index} started on the port {p_server}.")
    else:
        print(f"Server for index = {index} failed to start on the port {p_server}.")
        print(f"Check the log file at {server_log_file}")

def run_client(cmd_run_client, index):
    print(f"Running client for index = {index}.")
    client_process = subprocess.Popen(cmd_run_client, shell=True)
    client_process.wait()
TNRBs = [18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]

working_dir_server = "/home/changqi/ORAM/SGX_BT/SampleEnclave"
print(f"current working dir: {os.getcwd()}")
for idx, TNRB in enumerate(TNRBs):
    cmd_run_server_read_path_css = f"./../experimentalResults/CSH_SGX_STAR/readPath/breakDownCost/64KB/server/bin/csh_sgx_star_{TNRB} server"
    cmd_run_server_early_reshuffle_css = f"./../experimentalResults/CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/server/bin/csh_sgx_star_{TNRB} server"
    cmd_run_server_evict_path_css = f"./../experimentalResults/CSH_SGX_STAR/evictPath/breakDownCost/64KB/server/bin/csh_sgx_star_{TNRB} server"
    log_file_server_read_path_css = f"./../experimentalResults/CSH_SGX_STAR/readPath/breakDownCost/64KB/server/logs/log_{TNRB}.txt"
    log_file_server_early_reshuffle_css = f"./../experimentalResults/CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/server/logs/log_{TNRB}.txt"
    log_file_server_evict_path_css = f"./../experimentalResults/CSH_SGX_STAR/evictPath/breakDownCost/64KB/server/logs/log_{TNRB}.txt"
    working_dir_read_path_css = f"./../CSH_SGX_STAR/readPath/breakDownCost/64KB/client/bin"
    working_dir_early_reshuffle_css = f"./../CSH_SGX_STAR/earlyReshuffle/breakDownCost/64KB/client/bin"
    working_dir_evict_path_css = f"./../CSH_SGX_STAR/evictPath/breakDownCost/64KB/client/bin"
    log_file_client_read_path_css = f"../logs/log_{TNRB}.txt"
    log_file_client_early_reshuffle_css = f"../logs/log_{TNRB}.txt"
    log_file_client_evict_path_css = f"../logs/log_{TNRB}.txt"
    cmd_run_client_read_path_css = f"cd {working_dir_read_path_css} && ./csh_sgx_star_{TNRB} client_read_path" + f" >> {log_file_client_read_path_css} 2>&1"
    cmd_run_client_early_reshuffle_css = f"{working_dir_early_reshuffle_css} && ./csh_sgx_star_{TNRB} client_early_reshuffle" + f" >> {log_file_client_early_reshuffle_css} 2>&1"
    cmd_run_client_evict_path_css = f"{log_file_client_evict_path_css} && ./csh_sgx_star_{TNRB} client_eviction" + f" >> {log_file_client_evict_path_css} 2>&1"
    # cmd_run_client = ['./TriConvergeStar_TNRB_{}'.format(index), 'client_eviction']
    # cmd_run_client_read_path_css = ['./csh_sgx_star_{}'.format(TNRB), "client_read_path"]
    # cmd_run_client_early_reshuffle_css = ['./csh_sgx_star_{}'.format(TNRB), 'client_early_reshuffle' + f" >> {log_file_client_early_reshuffle_css} 2>&1"]
    # cmd_run_client_evict_path_css = ['./csh_sgx_star_{}'.format(TNRB), 'client_eviction' + f" >> {log_file_client_evict_path_css} 2>&1"]
    with manage_ssh_connection(SSH_HOST, SSH_USER, SSH_PASS) as ssh_server:
        stop_remote_process(ssh_server, p_server)

        # Run server and third_party on the remote server
        run_server(ssh_server, TNRB, working_dir_server, cmd_run_server_read_path_css, log_file_server_read_path_css)

        # Run client after server and third_party are started
        run_client(cmd_run_client_read_path_css, TNRB)

        stop_remote_process(ssh_server, p_server)

        if idx == 0:
            run_server(ssh_server, TNRB, working_dir_server, cmd_run_server_early_reshuffle_css, log_file_server_early_reshuffle_css)
            run_client(cmd_run_client_early_reshuffle_css, TNRB)
            stop_remote_process(ssh_server, p_server)
        
        run_server(ssh_server, TNRB, working_dir_server, cmd_run_server_evict_path_css, log_file_server_evict_path_css)
        run_client(cmd_run_client_evict_path_css, TNRB)
        
        stop_remote_process(ssh_server, p_server)


# print("All processes completed.")
