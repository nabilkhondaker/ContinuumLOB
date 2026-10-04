# System Overview

Continuum-LOB separates:

1. **High-fidelity reference** — discrete LOB + finite-volume continuum solver
2. **Learned operator** — FNO / DeepONet trained on solver trajectories
3. **Control** — execution / market-making environments using either solver or surrogate
4. **Validation** — conservation, convergence, rollout, sim-to-sim transfer

No single monolithic class owns the full pipeline. Components communicate through
explicit state objects (`ContinuumState`, `BookSnapshot`, `ExecutionState`).
