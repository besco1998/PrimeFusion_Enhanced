#!/usr/bin/env python3
import pandas as pd
import os

# --- Configuration ---
X86_RESULTS_PATH = "results/raw/2026-01-16_074735/raw_results.csv"
ARM_RESULTS_PATH = "results/raw/2026-01-30_222420/raw_results.csv"

BLOCK_INTERVAL_SEC = 1.0  # Time budget for one block
MAX_BLOCK_SIZE_BYTES = 1024 * 1024  # 1 MB soft limit

def load_data(path, platform_name):
    if not os.path.exists(path):
        print(f"Error: {path} not found.")
        return None
    df = pd.read_csv(path)
    # Filter for Standard variants only for cleanest comparison
    df = df[df['Variant'] == 'Standard'].copy()
    df['Platform'] = platform_name
    return df

def analyze_capacity():
    print("Loading Data...")
    df_x86 = load_data(X86_RESULTS_PATH, "x86 (i7)")
    df_arm = load_data(ARM_RESULTS_PATH, "ARM (RPi4)")
    
    if df_x86 is None or df_arm is None:
        print("Aborting analysis due to missing files.")
        return

    combined = pd.concat([df_x86, df_arm])

    # === Model Definitions ===
    # T_total(k) = k * t_enc_single
    # Constraint: T_total(k) <= BLOCK_INTERVAL_SEC (1.0s)
    # Max TPS = 1 second / t_enc_single (in seconds)
    
    print("\n=== SYSTEM CAPACITY MODEL (Analytic Projection) ===")
    print(f"Assumptions: Block Interval = {BLOCK_INTERVAL_SEC}s, Linear Scaling")
    print("-" * 80)
    print(f"{'Platform':<15} | {'Format':<10} | {'Encode(us)':<12} | {'Max TPS':<12} | {'Max Block(msgs)':<15}")
    print("-" * 80)

    results = []

    for _, row in combined.iterrows():
        plat = row['Platform']
        fmt = row['Format']
        t_enc_us = row['AvgEncode(us)']
        t_dec_us = row['AvgDecode(us)']
        
        # Max Throughput (Transactions Per Second) allowed by CPU budget
        # TPS = 1,000,000 / t_enc_us
        max_tps = 1_000_000 / t_enc_us if t_enc_us > 0 else 0
        
        # Max Messages fit in 1 Block Interval (assuming encoding is the bottleneck)
        max_msgs_per_block = int(max_tps * BLOCK_INTERVAL_SEC)
        
        print(f"{plat:<15} | {fmt:<10} | {t_enc_us:<12.2f} | {int(max_tps):<12,} | {max_msgs_per_block:<15,}")

        results.append({
            "Platform": plat,
            "Format": fmt,
            "Encode_us": t_enc_us,
            "Max_TPS": max_tps
        })

    print("-" * 80)
    print("\n=== Observations ===")
    
    # Calculate Ratio
    cbor_arm = next(r for r in results if r['Platform'] == 'ARM (RPi4)' and r['Format'] == 'CBOR')
    json_arm = next(r for r in results if r['Platform'] == 'ARM (RPi4)' and r['Format'] == 'JSON')
    
    ratio = cbor_arm['Max_TPS'] / json_arm['Max_TPS']
    print(f"1. On RPi4, switching from JSON to CBOR increases Max Throughput by {ratio:.1f}x.")
    print(f"   (From {int(json_arm['Max_TPS']):,} to {int(cbor_arm['Max_TPS']):,} msgs/sec)")
    
    msgpack_arm = next(r for r in results if r['Platform'] == 'ARM (RPi4)' and r['Format'] == 'MSGPACK')
    print(f"2. MsgPack (DOM) suffers on ARM. Encode Capacity: {int(msgpack_arm['Max_TPS']):,} TPS.")

if __name__ == "__main__":
    analyze_capacity()
