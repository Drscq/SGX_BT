# SampleEnclave Project

## Purpose of SampleEnclave

The project demonstrates several fundamental usages of Intel(R) Software Guard Extensions (Intel(R) SGX) SDK:
- Initializing and destroying an enclave
- Creating ECALLs or OCALLs
- Calling trusted libraries inside the enclave
- Using multiple threads to demonstrate host and enclave interaction
- Passing host memory to the enclave with `[user_check]`

---

## How to Build/Execute the Sample Code

### 1. Install Intel(R) SGX SDK for Linux* OS

### 2. Enclave Test Key (Two Options)
- **Option A**: Install OpenSSL first, then the project will generate a test key `<Enclave_private_test.pem>` automatically when you build the project.
- **Option B**: Rename your test key (3072-bit RSA private key) to `<Enclave_private_test.pem>` and place it in the `<Enclave>` folder.

### 3. Set Up the Environment
```sh
source ${sgx-sdk-install-path}/environment
```

### 4. Build the Project

#### **Hardware Mode, Debug Build**
```sh
make
```

#### **Enclave with mitigations for indirects and returns only**
```sh
make MITIGATION-CVE-2020-0551=CF
```

#### **Enclave with full mitigation**
```sh
make MITIGATION-CVE-2020-0551=LOAD
```

#### **Hardware Mode, Pre-release Build**
```sh
make SGX_PRERELEASE=1 SGX_DEBUG=0
```

#### **Simulation Mode, Debug Build**
```sh
make SGX_MODE=SIM
```

### 5. Execute the Binary
```sh
./app
```

### 6. Clean Before Switching Build Mode
```sh
make clean
```

## Multi-threaded Host and Enclave Interaction

This project demonstrates launching **two threads**:
1. **`thread_server` (host thread)**:
   - Generates an array with random integers.
   - Passes the array to the enclave using `[user_check]`.
   - Waits for the enclave to sort the array.
   - Adds all the even-indexed elements of the sorted array.

2. **`thread_enclave` (enclave thread)**:
   - Accepts the array from the host.
   - Sorts the array inside the enclave using `qsort()`.
   - Uses `[user_check]` to directly access host memory without copying.

### Steps to Test Multi-threaded Execution
- **Build and run** the project as usual.
- The host thread (`thread_server`) will generate random numbers.
- The enclave thread (`thread_enclave`) will **sort** the numbers inside SGX.
- The host will **sum** the even-indexed elements of the sorted array and **print** the result.

---

## Configuration Parameters

### Thread Control Parameters
- `TCSMaxNum`, `TCSNum`, `TCSMinPool`
  - These parameters determine whether a thread will be **created dynamically** when there are no available threads.

### Stack Parameters
- `StackMaxSize`, `StackMinSize`
  - `StackMinSize`: Amount of stack available when a thread is created.
  - `StackMaxSize`: Maximum stack available to the thread.
  - **Static threads** only use `StackMaxSize`.

### Heap Parameters
- `HeapMaxSize`, `HeapInitSize`, `HeapMinSize`
  - `HeapMinSize`: Initial heap size when the enclave starts.
  - `HeapMaxSize`: Maximum heap size, dynamically expandable at runtime.

---

## Sample Configuration Files for the Enclave

| Config File  | Description |
|-------------|------------|
| `config.01.xml` | No dynamic threads, no dynamic heap expansion. |
| `config.02.xml` | No dynamic threads, but heap expansion allowed. |
| `config.03.xml` | Dynamic threads enabled, no stack expansion. |
| `config.04.xml` | Dynamic threads enabled, stack expands dynamically. |
| `config.05.xml` | User-defined memory region (SGX2 only). |

---

## Launch Token Initialization

If using `libsgx-enclave-common` or `sgxpsw` under version **2.4**, an initialized variable `launch_token` needs to be passed as the **third parameter** of `sgx_create_enclave`:

```c
sgx_launch_token_t launch_token = {0};
sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, launch_token, NULL, &global_eid, NULL);
```

---

## Secure Host Memory Access with `[user_check]`

- The `[user_check]` attribute in **EDL** allows the enclave to **directly access untrusted host memory**.
- **Warning:** `[user_check]` is unsafe if not used properly, as it does **not** enforce memory copying.
- **Ensure** the enclave verifies pointer validity before using `[user_check]` data to prevent security risks.

### **EDL Declaration**
```c
enclave {
    trusted {
        public void ecall_sort_array([user_check] int* arr, size_t size);
    };
};
```

### **Memory Validation in Enclave Code**
```cpp
#include "Enclave_t.h"
#include <algorithm>

void ecall_sort_array(int* arr, size_t size) {
    // Ensure the pointer is valid before accessing
    if (!sgx_is_outside_enclave(arr, size * sizeof(int))) {
        return;  // Invalid memory, reject access
    }

    // Sort the array in-place
    std::sort(arr, arr + size);
}
```

### **Expected Execution Flow**
1. **Host thread (`thread_server`)**:
   - Allocates and fills an array with **random numbers**.
   - Calls `ecall_sort_array()` to send the array **directly to the enclave**.

2. **Enclave thread (`thread_enclave`)**:
   - Sorts the array **inside SGX**.
   - Returns the sorted array to the host **without copying**.

3. **Host thread (`thread_server`)**:
   - Iterates through **even-indexed elements** of the sorted array.
   - Computes and prints the **sum of even-indexed elements**.

---

## Example Output
```
Original array: [23, 8, 15, 42, 7, 3, 56, 21]
Sorted array inside Enclave: [3, 7, 8, 15, 21, 23, 42, 56]
Sum of even-indexed elements: 3 + 8 + 21 + 42 = 74
```

---

## Summary
- **Multi-threading**: Host and enclave threads interact via ECALLs.
- **Direct Memory Access**: `[user_check]` allows **in-place modification** of host memory.
- **Secure Implementation**: `sgx_is_outside_enclave()` verifies memory before enclave access.
- **Dynamic Sorting**: The enclave sorts data securely while minimizing **unnecessary memory copies**.

---

## Next Steps
- **Enable Secure Communication**: Encrypt memory regions between the host and enclave.
- **Add Multi-threaded OCALLs**: Enhance security with enclave-to-host **asynchronous communication**.
- **Optimize Performance**: Use **SIMD optimizations** for faster sorting inside the enclave.

---

This README has been updated to include **multi-threading and `[user_check]` integration**. Follow the **build and execution steps** to test the new features. 🚀