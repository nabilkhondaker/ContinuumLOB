#pragma once
/// \file exceptions.hpp
/// Explicit error types for numerical, configuration and runtime failures.

#include <stdexcept>
#include <string>

namespace continuum_lob {

class ContinuumLobError : public std::runtime_error {
public:
  explicit ContinuumLobError(const std::string& msg) : std::runtime_error(msg) {}
};

class ConfigError : public ContinuumLobError {
public:
  explicit ConfigError(const std::string& msg) : ContinuumLobError("Config: " + msg) {}
};

class NumericalError : public ContinuumLobError {
public:
  explicit NumericalError(const std::string& msg) : ContinuumLobError("Numerical: " + msg) {}
};

class CFLViolation : public NumericalError {
public:
  explicit CFLViolation(const std::string& msg) : NumericalError("CFL violation: " + msg) {}
};

class InvalidDensity : public NumericalError {
public:
  explicit InvalidDensity(const std::string& msg) : NumericalError("Invalid density: " + msg) {}
};

class DimensionMismatch : public ContinuumLobError {
public:
  explicit DimensionMismatch(const std::string& msg) : ContinuumLobError("Dimension mismatch: " + msg) {}
};

class ActionConstraintViolation : public ContinuumLobError {
public:
  explicit ActionConstraintViolation(const std::string& msg)
      : ContinuumLobError("Action constraint: " + msg) {}
};

}  // namespace continuum_lob
