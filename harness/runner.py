#!/usr/bin/env python3
import os
import sys
import subprocess
import time
import datetime
import platform
import glob

# ==============================================================================
# Configuration
# ==============================================================================
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
BUILD_DIR = os.path.join(PROJECT_ROOT, "build")
BIN_DIR = os.path.join(BUILD_DIR, "bin")
RESULTS_DIR = os.path.join(PROJECT_ROOT, "results", "raw")

ITERATIONS = int(os.environ.get("BENCHMARK_ITERATIONS", "1000000"))

MATRIX = {
    "libpf_json.so": ["Standard", "Canonical", "Base64"],
    "libpf_cbor.so": ["Standard", "StringKeys"],
    "libpf_msgpack.so": ["Standard", "StringKeys"],
    "libpf_protobuf.so": ["Standard"]
}

# ==============================================================================
# Helper Functions
# ==============================================================================
def get_env_info():
    info = {
        "timestamp": datetime.datetime.now().isoformat(),
        "node": platform.node(),
        "system": platform.system(),
        "release": platform.release(),
        "version": platform.version(),
        "machine": platform.machine(),
        "processor": platform.processor(),
    }
    
    # helper to run command safely
    def get_cmd(cmd):
        try:
            return subprocess.check_output(cmd, shell=True).decode().strip()
        except:
            return "unknown"

    info["cpu_model"] = get_cmd("grep -m1 'model name' /proc/cpuinfo | cut -d: -f2")
    info["cpu_cores"] = get_cmd("nproc")
    info["gcc_version"] = get_cmd("gcc --version | head -n1")
    info["cmake_version"] = get_cmd("cmake --version | head -n1")
    info["kernel_version"] = get_cmd("uname -srv")
    
    # Try to grab library versions if possible (pkg-config)
    info["libcbor_version"] = get_cmd("pkg-config --modversion libcbor")
    info["msgpack_version"] = "6.1.0 (Fetched)" 
    info["protobuf_version"] = get_cmd("pkg-config --modversion protobuf")
    info["rapidjson_version"] = get_cmd("pkg-config --modversion RapidJSON")

    return info

def parse_metrics(output_bytes):
    metrics = {}
    lines = output_bytes.decode().splitlines()
    for line in lines:
        if "=" in line:
            key, val = line.split("=", 1)
            metrics[key.strip()] = val.strip()
    return metrics

# ==============================================================================
# Main Logic
# ==============================================================================
def run_benchmark_set(plugin_name, variant, run_dir):
    found_plugins = glob.glob(os.path.join(BUILD_DIR, "**", plugin_name), recursive=True)
    if not found_plugins:
        print(f"⚠️  Plugin {plugin_name} not found. Skipping.")
        return False
    
    plugin_path = found_plugins[0]
    
    # 1. Run Time Benchmark
    print(f"   ⏳ [Time]   {plugin_name} [{variant}] ...", end="", flush=True)
    # Using taskset -c 0 to pin to single core
    cmd_time = ["taskset", "-c", "0", os.path.join(BIN_DIR, "pf_runner_time"), plugin_path, variant, str(ITERATIONS), run_dir]
    try:
        out_time = subprocess.check_output(cmd_time, stderr=subprocess.STDOUT)
        time_metrics = parse_metrics(out_time)
        print(" Done.")
    except Exception as e:
        print(f" Failed: {e}")
        return False

    # 2. Run Memory Benchmark
    print(f"   🧠 [Memory] {plugin_name} [{variant}] ...", end="", flush=True)
    cmd_mem = ["taskset", "-c", "0", os.path.join(BIN_DIR, "pf_runner_memory"), plugin_path, variant, str(ITERATIONS)]
    try:
        out_mem = subprocess.check_output(cmd_mem, stderr=subprocess.STDOUT)
        mem_metrics = parse_metrics(out_mem)
        print(" Done.")
    except Exception as e:
        print(f" Failed: {e}")
        return False
        
    # 3. Aggregate
    # Format, Variant, Iterations, Time, Encode, Decode, Size, RSS, Heap
    
    csv_path = os.path.join(run_dir, "raw_results.csv")
    write_header = not os.path.exists(csv_path)
    
    with open(csv_path, "a") as f:
        if write_header:
            f.write("Format,Variant,Iterations,TotalTime(ms),AvgEncode(us),AvgDecode(us),Size(bytes),PeakRSS(KB),MallocDeltaCold(bytes),MallocDeltaWarm(bytes)\n")
        
        # Determine Format Name (Strip libpf_ and .so)
        fmt = plugin_name.replace("libpf_", "").replace(".so", "").upper()
        
        row = [
            fmt,
            variant,
            str(ITERATIONS),
            time_metrics.get("TOTAL_TIME_MS", "0"),
            time_metrics.get("AVG_ENCODE_US", "0"),
            time_metrics.get("AVG_DECODE_US", "0"),
            mem_metrics.get("SERIALIZED_SIZE", "0"),
            mem_metrics.get("PEAK_RSS_KB", "0"),
            mem_metrics.get("MALLOC_DELTA_COLD", "0"),
            mem_metrics.get("MALLOC_DELTA_WARM", "0")
        ]
        f.write(",".join(row) + "\n")
        
    return True

def main():
    print("=================================================================")
    print("     PrimeFusion Enhanced - Dual-Runner Harness")
    print("=================================================================")
    
    ITERATIONS = int(os.environ.get("BENCHMARK_ITERATIONS", "100000")) # Lower default for dev
    run_dir = os.path.join(RESULTS_DIR, datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S"))
    os.makedirs(run_dir, exist_ok=True)
    print(f"Results: {run_dir}")
    print(f"Iterations: {ITERATIONS}")
    
    # Write Metadata
    meta = get_env_info()
    with open(os.path.join(run_dir, "metadata.txt"), "w") as f:
        for k, v in meta.items():
            f.write(f"{k}: {v}\n")
        f.write(f"iterations: {ITERATIONS}\n")
    
    total = 0
    success = 0
    
    for lib, variants in MATRIX.items():
        for variant in variants:
            if run_benchmark_set(lib, variant, run_dir):
                success += 1
            total += 1
            
    print(f"Done. {success}/{total} completed.")

if __name__ == "__main__":
    main()
