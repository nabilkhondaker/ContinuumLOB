# Model Risk

## Continuum approximation
The mapping from discrete orders to continuous fields loses queue position,
order-priority, and tick-grid structure. Control policies that depend on exact
queue rank may not transfer.

## Constitutive misspecification
Pressure and viscosity laws are parametric analogies. Different parameter
combinations can produce similar macroscopic observables (identifiability risk).

## Neural approximation
FNO spectral truncation and finite training distributions limit extrapolation.
Rollout error accumulates; conservation is not guaranteed by the network alone.

## RL and surrogate exploitation
Policies trained against a surrogate may exploit numerical or approximation
artifacts. Always evaluate against the high-fidelity finite-volume environment
(sim-to-sim).

## Synthetic market bias
Event intensities, regimes, and impact responses are synthetic. No claim is
made about real exchange dynamics or profitability.
