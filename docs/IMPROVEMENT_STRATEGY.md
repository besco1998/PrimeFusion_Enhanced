# Strategic Improvement Plan (2026-01-31)

**Source:** Expert Feedback (ChatGPT)
**Objective:** Elevate the project from "Code Benchmark" to "System-Level Research" with minimal risk to stability.

## 1. Risk/Reward Analysis of Suggestions

| Suggestion | Impact | Implementation Risk | Status |
| :--- | :--- | :--- | :--- |
| **1. System-Level Capacity Model** | 🔴 **High** (The "So What?") | 🟢 **Zero** (New Analysis Script) | **Recommended First Step** |
| **2. Benchmark "Blocks" (Vector)** | 🟡 Medium (Scale) | 🔴 **High** (Requires C++ Refactor) | *Defer (Phase 2)* |
| **3. Determinism/Canonicalization** | 🟡 Medium (Correctness) | 🟢 **Low** (Doc + small tweaks) | **Recommended** |
| **4. Broaden Payloads** | ⚪ Low (Generalizability) | 🟡 Medium (New Proto/Data) | *Defer* |
| **5. Reviewer-Proofing (Pinning)** | 🟡 Medium (Rigor) | 🟢 **Low** (Runner.py tweak) | **Recommended** |
| **6. Takeaway Table** | 🟡 Medium (Conclusion) | 🟢 **Zero** (Markdown update) | **Recommended** |

---

## 2. Proposed "Minimal Hazard" Roadmap

We will focus on **Additive Improvements** that do not touch the core C++ encoding loops initially.

### Step 1: Analytical Modeling (The "So What?")
*   **Action:** Create `analysis/capacity_model.py`.
*   **Logic:** Take the *existing* micro-benchmark results (µs/msg) and mathematically project the "Block Feasibility" using the formulas provided:
    *   `t_total(k) = k * t_encode_single` (Linear projection for now)
    *   Calculate max TPS (Transactions Per Second) for RPi4 vs x86.
*   **Benefit:** Generates the "System-Level" graphs without rewriting the C++ harness.

### Step 2: Methodology Rigor (The "Reviewer-Proofing")
*   **Action:** Modify `harness/runner.py`.
*   **Improvement:** Add `--cpu-pin` argument to use `taskset`.
*   **Benefit:** Reduces variance on RPi4. Safe because it's just a python wrapper change.

### Step 3: Synthesis & Reporting
*   **Action:** Update `DEV_KNOWLEDGE_BASE.md`.
*   **Content:**
    *   Add the "Takeaway Table" (Decision Matrix).
    *   Add a section on "Projected System Capacity" (from Step 1).
    *   Explicitly discuss Determinism (JCS etc.) as a design consideration.

---

## 3. Deferred High-Risk Items
*   **Block Benchmarking (C++):** Changing `encode(Payload)` to `encode(vector<Payload>)` is a breaking change for `IBenchmark`.
    *   *Alternative:* We can simulate this in the *Analytic Model* first. If the model is compelling, we don't strictly need to write the C++ code to prove the trend (O(N) is usually O(N)).
