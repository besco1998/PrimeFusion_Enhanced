# Remote Execution Plan: Raspberry Pi 4 (ARM64)

**Objective:** Deploy, build, and run the PrimeFusion Enhanced benchmark suite on a remote Raspberry Pi 4 to gather "Honest" Low-Power metrics.

**Target Device:**
- **IP:** `192.168.1.14`
- **User:** `besco1998` (Assumed matching local user) or `pi`
- **Password:** `123456`

## 1. Strategy: "Push, Run, Pull"
Since the project relies on a portable directory structure without Docker, we will use `rsync` to mirror the codebase and `ssh` to drive the execution.

### Phase 1: Preparation (Local)
1.  **Verify Connectivity:** Ping the device (Confirmed).
2.  **Clean Build Artifacts:** Ensure we don't transfer x86 binaries to ARM. Run `make clean` or exclude `build/`.
3.  **Generate Deployment Script:** Create `tools/deploy_to_pi.sh` to automate the tedious `rsync`/`ssh` commands.

### Phase 2: Deployment (Transfer)
*   **Command:** `rsync -avz --exclude 'build' --exclude '.git' ./ <USER>@192.168.1.14:~/simulations/PrimeFusion_Enhanced/`
*   **Goal:** Create an identical source tree on the RPi.

### Phase 3: Remote Setup (One-Time)
*   **Command:** `ssh <USER>@192.168.1.14 "bash ~/simulations/PrimeFusion_Enhanced/tools/install_dependencies.sh"`
*   **Goal:** Install `cmake`, `g++`, `libprotobuf-dev`, etc. on the RPi.

### Phase 4: Remote Build (ARM64)
*   **Commands:**
    ```bash
    cd ~/simulations/PrimeFusion_Enhanced
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug  # Force -O0 for honest bench
    make -j4
    ```

### Phase 5: Remote Execution
*   **Command:** `python3 ../harness/runner.py`
*   **Goal:** Run the benchmark harness using the compiled ARM binaries. The harness is platform-agnostic.

### Phase 6: Harvest Results
*   **Command:** `scp -r <USER>@192.168.1.14:~/simulations/PrimeFusion_Enhanced/results/raw/* ./results/raw/`
*   **Goal:** Sync the CSV files back to the local machine for comparison.

## 2. Automation Script (`tools/deploy_to_pi.sh`)
I will create a script that encapsulates these steps.
**Usage:** `bash tools/deploy_to_pi.sh [REMOTE_USER] [REMOTE_IP]`
**Default:** `besco1998@192.168.1.14`

## 3. Comparison Plan
Once results are retrieved:
1.  The `results/raw/` directory will contain timestamps from both x86 and ARM runs.
2.  `metadata.txt` in each folder will identify `x86_64` vs `aarch64`.
3.  We can script a comparison (Phase 4 Visualization) to plot these side-by-side.
