# Continuum Model Motivation

Limit-order books are discrete, event-driven systems. Continuum-LOB investigates
whether a coarse-grained density–velocity description can:

- support conservative numerical schemes,
- generate structured training data for operator learning,
- provide low-latency surrogates for control experiments.

The continuum representation is deliberately coarse. It is not intended to
replace discrete event simulation for queue-level microstructure questions.
