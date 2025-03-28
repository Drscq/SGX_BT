import csv
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def plot_csv_data(csv_path, output_image='elapsed_durations.png', title='Elapsed Durations', xlabel='Iterations', ylabel='Time (ns)'):
    openssl_times = []
    ntl_times = []
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        for row in reader:
            openssl_times.append(int(row[0]))
            ntl_times.append(int(row[1]))
    plt.plot(openssl_times, label='OpenSSL')
    plt.plot(ntl_times, label='NTL')
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid()
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_image)
    plt.close()

if __name__ == '__main__':
    plot_csv_data('../build/elapsed_durations.csv', 'elapsed_durations.png', 'Delay OpenSSL vs NTL on Banana', xlabel='Iterations', ylabel='Time (ns)')
    plot_csv_data('../build/elapsed_durations_parallel.csv', 'elapsed_durations_parallel.png', 'Delay OpenSSL vs NTL in Parallel on Banana', xlabel='Threads Nums', ylabel='Time (ns)')