#!/bin/bash
# Usage: ./tools/deploy_to_pi.sh [USER] [IP]
# Default: pi 192.168.1.14

REMOTE_USER=${1:-pi}
REMOTE_IP=${2:-192.168.1.14}
REMOTE_DIR="~/simulations/PrimeFusion_Enhanced"

echo "========================================================"
echo "   PrimeFusion Enhanced - Remote Deployment (RPi4)"
echo "   Target: $REMOTE_USER@$REMOTE_IP"
echo "========================================================"

# 1. Sync Source Code (Excluding build artifacts and git)
echo "Step 1: Syncing Codebase..."
rsync -avz --exclude 'build' --exclude '.git' --exclude 'results' ./ $REMOTE_USER@$REMOTE_IP:$REMOTE_DIR/

# 2. Remote Dependencies
echo "Step 2: Install Remote Dependencies..."
ssh -t $REMOTE_USER@$REMOTE_IP "sudo apt-get update; sudo apt-get install -y build-essential pkg-config cmake git python3 python3-pip libprotobuf-dev protobuf-compiler libcbor-dev libmsgpack-dev rapidjson-dev"

# 3. Remote Build (Clean Debug Build)
echo "Step 3: Building on ARM..."
ssh -t $REMOTE_USER@$REMOTE_IP "cd $REMOTE_DIR && mkdir -p build && cd build && rm -rf * && cmake .. -DCMAKE_BUILD_TYPE=Debug && make -j4"

# 4. Remote Execution
echo "Step 4: Running Harness..."
ssh -t $REMOTE_USER@$REMOTE_IP "cd $REMOTE_DIR && python3 harness/runner.py"

# 5. Retrieve Results
echo "Step 5: Retrieving Results..."
mkdir -p results/raw
scp -r $REMOTE_USER@$REMOTE_IP:$REMOTE_DIR/results/raw/* ./results/raw/

echo "========================================================"
echo "   Success! Data synced to ./results/raw/"
echo "========================================================"
