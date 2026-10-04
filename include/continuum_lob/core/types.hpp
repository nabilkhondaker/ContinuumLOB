#pragma once
/// \file types.hpp
/// Core scalar and index types for Continuum-LOB.
/// Author: Nabil Khondaker

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <span>
#include <stdexcept>
#include <cmath>
#include <limits>

namespace continuum_lob {

using Index = std::int64_t;
using Size  = std::size_t;
using Real  = double;

constexpr Real kPi = 3.14159265358979323846;
constexpr Real kEps = std::numeric_limits<Real>::epsilon();
constexpr Real kInf = std::numeric_limits<Real>::infinity();

/// Side of the order book.
enum class Side : std::uint8_t {
  Bid = 0,
  Ask = 1
};

/// Market event type.
enum class EventType : std::uint8_t {
  LimitOrder = 0,
  Cancel     = 1,
  MarketOrder= 2,
  Modify     = 3,
  Trade      = 4
};

/// Boundary condition type for the continuum domain.
enum class BCType : std::uint8_t {
  Reflecting = 0,
  Absorbing  = 1,
  Periodic   = 2,
  Fixed      = 3,
  ZeroFlux   = 4
};

/// Constitutive pressure model.
enum class PressureModel : std::uint8_t {
  Linear    = 0,
  Nonlinear = 1,
  Saturating= 2
};

/// Viscosity / impact model.
enum class ViscosityModel : std::uint8_t {
  Newtonian      = 0,
  ShearThinning  = 1,
  ShearThickening= 2,
  Generalized    = 3
};

/// Kernel used for coarse-graining.
enum class KernelType : std::uint8_t {
  Gaussian       = 0,
  CompactSupport = 1,
  Triangular     = 2,
  Histogram      = 3
};

inline const char* to_string(Side s) {
  return s == Side::Bid ? "bid" : "ask";
}

inline const char* to_string(EventType e) {
  switch (e) {
    case EventType::LimitOrder:  return "limit";
    case EventType::Cancel:      return "cancel";
    case EventType::MarketOrder: return "market";
    case EventType::Modify:      return "modify";
    case EventType::Trade:       return "trade";
  }
  return "unknown";
}

}  // namespace continuum_lob
