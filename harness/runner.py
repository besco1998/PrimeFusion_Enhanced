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
import argparse

# ... (Configuration section remains the same) ...

# ==============================================================================
# Main Logic
# ==============================================================================
# ==============================================================================
# Main Logic
# ==============================================================================
def run_benchmark_set(runner_bin, plugin_path, variant, run_dir, cpu_pin):
    if not os.path.exists(plugin_path):
        print(f"⚠️  Plugin {plugin_path} not found. Skipping.")
        return False
        
    plugin_name = os.path.basename(plugin_path)
    
    # 1. Run Time Benchmark (Unified Runner Default)
    print(f"   ⏳ [Time]   {plugin_name} [{variant}] ...", end="", flush=True)
    cmd_time = ["taskset", "-c", str(cpu_pin), runner_bin, plugin_path, variant, str(ITERATIONS)]
    try:
        out_time = subprocess.check_output(cmd_time, stderr=subprocess.STDOUT)
        time_metrics = parse_metrics(out_time)
        print(" Done.")
    except Exception as e:
        print(f" Failed: {e}")
        return False

    # 2. Run Memory Benchmark (Unified Runner --memory)
    print(f"   🧠 [Memory] {plugin_name} [{variant}] ...", end="", flush=True)
    cmd_mem = ["taskset", "-c", str(cpu_pin), runner_bin, "--memory", plugin_path, variant, str(ITERATIONS)]
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
            f.write("Scenario,Format,Variant,Iterations,TotalTime(ms),AvgEncode(us),AvgDecode(us),Size(bytes),PeakRSS(KB),MallocDeltaCold(bytes),MallocDeltaWarm(bytes)\n")
        
        # Determine Format Name
        # libpf_json.so -> JSON
        # libpf_json_battery.so -> JSON
        base = plugin_name.replace("libpf_", "").replace(".so", "")
        # base is e.g. "json", "json_battery"
        # Split by underscore?
        if "_" in base and not base in ["msgpack", "protobuf"]: # care! msgpack has no underscore but might if msg_pack? No.
             # e.g. json_battery -> format=json, scenario=battery
             # But msgpack -> format=msgpack
             parts = base.split("_")
             fmt = parts[0].upper()
             # But wait, cbor_battery -> CBOR
        else:
             fmt = base.upper()

        scenario = "Unknown"
        if "battery" in plugin_name: scenario = "Battery"
        elif "odometry" in plugin_name: scenario = "Odometry"
        elif "attitude" in plugin_name: scenario = "Attitude"
        elif "global_position" in plugin_name: scenario = "GlobalPosition"
        elif "gps_block" in plugin_name: scenario = "GPSBlock"
        elif "status" in plugin_name: scenario = "Status"
        else: scenario = "GPSRaw"

        row = [
            scenario,
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
    parser = argparse.ArgumentParser(description="PrimeFusion Enhanced Benchmark Runner")
    parser.add_argument("--cpu-pin", type=str, default="0", help="CPU core(s) to pin the benchmark process to (default: 0)")
    args = parser.parse_args()

    print("=================================================================")
    print("     PrimeFusion Enhanced - Matrix Runner")
    print(f"     CPU Pinning: Core {args.cpu_pin}")
    print("=================================================================")
    
    global ITERATIONS
    ITERATIONS = int(os.environ.get("BENCHMARK_ITERATIONS", "100000")) 
    run_dir = os.path.join(RESULTS_DIR, datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S"))
    os.makedirs(run_dir, exist_ok=True)
    print(f"Results: {run_dir}")
    print(f"Iterations: {ITERATIONS}")
    
    # Write Metadata
    meta = get_env_info()
    meta["cpu_pin"] = args.cpu_pin
    with open(os.path.join(run_dir, "metadata.txt"), "w") as f:
        for k, v in meta.items():
            f.write(f"{k}: {v}\n")
        f.write(f"iterations: {ITERATIONS}\n")
    
    total = 0
    success = 0
    
    # Define Scenarios
    # Scenario -> Runner Binary
    SCENARIOS = {
        "GPSRaw": "pf_runner_gps_raw",
        "Battery": "pf_runner_battery",
        "Odometry": "pf_runner_odometry",
        "Attitude": "pf_runner_attitude",
        "GlobalPosition": "pf_runner_global_position",
        "Status": "pf_runner_status",
        "GPSBlock": "pf_runner_gps_block"
    }

    # Define Formats and Variants
    FORMATS = {
        "json": ["Standard", "Canonical", "Base64"],
        "cbor": ["Standard"], # "StringKeys" removed for simplicity or add back? Add back if supported.
        "msgpack": ["Standard"],
        "protobuf": ["Standard"]
    }
    # Note: Variants support depends on plugin implementation. 
    # Current implementations mostly ignore variants except JSON?
    # Let's keep "Standard" for all for now to ensure success.

    for s_name, runner_name in SCENARIOS.items():
        runner_bin = os.path.join(BIN_DIR, runner_name)
        if not os.path.exists(runner_bin):
            print(f"⚠️  Runner {runner_name} not found. Skipping Scenario {s_name}.")
            continue
            
        print(f"\n--- Scenario: {s_name} ---")
        
        for fmt, variants in FORMATS.items():
            # Construct plugin name
            # GPSRaw: libpf_json.so
            # Battery: libpf_json_battery.so
            suffix = ""
            if s_name != "GPSRaw":
                 # Convert CamelCase to snake_case? 
                 # Actually names are: battery, odometry, attitude, global_position, status, gps_block
                 # Map s_name to suffix
                 if s_name == "Battery": suffix = "_battery"
                 elif s_name == "Odometry": suffix = "_odometry"
                 elif s_name == "Attitude": suffix = "_attitude"
                 elif s_name == "GlobalPosition": suffix = "_global_position"
                 elif s_name == "Status": suffix = "_status"
                 elif s_name == "GPSBlock": suffix = "_gps_block"
            
            lib_name = f"libpf_{fmt}{suffix}.so"
            # Find plugin in build dir (recursive glob)
            found_plugins = glob.glob(os.path.join(BUILD_DIR, "**", lib_name), recursive=True)
            
            if not found_plugins:
                print(f"   ⚠️  Plugin {lib_name} not found.")
                continue
                
            plugin_path = found_plugins[0]
            
            for variant in variants:
                 if fmt != "json" and variant != "Standard": continue # Only JSON has variants impl currently?
                 
                 if run_benchmark_set(runner_bin, plugin_path, variant, run_dir, args.cpu_pin):
                     success += 1
                 total += 1

    print(f"\nDone. {success}/{total} completed.")

if __name__ == "__main__":
    main()
