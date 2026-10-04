#pragma once
/// \file strong_types.hpp
/// Strong typedefs to avoid mixing price, quantity, density, etc.

#include "continuum_lob/core/types.hpp"
#include <cmath>
#include <compare>

namespace continuum_lob {

template <typename Tag>
struct StrongReal {
  Real value{0.0};

  constexpr StrongReal() = default;
  explicit constexpr StrongReal(Real v) : value(v) {}

  constexpr Real get() const { return value; }
  explicit constexpr operator Real() const { return value; }

  constexpr auto operator<=>(const StrongReal&) const = default;

  constexpr StrongReal operator+(StrongReal o) const { return StrongReal{value + o.value}; }
  constexpr StrongReal operator-(StrongReal o) const { return StrongReal{value - o.value}; }
  constexpr StrongReal operator*(Real s) const { return StrongReal{value * s}; }
  constexpr StrongReal operator/(Real s) const { return StrongReal{value / s}; }
  constexpr StrongReal& operator+=(StrongReal o) { value += o.value; return *this; }
  constexpr StrongReal& operator-=(StrongReal o) { value -= o.value; return *this; }
};

struct PriceTag {};
struct QtyTag {};
struct DensityTag {};
struct VelocityTag {};
struct PressureTag {};
struct StressTag {};
struct InventoryTag {};
struct LatencyTag {};
struct TimeTag {};

using Price     = StrongReal<PriceTag>;
using Quantity  = StrongReal<QtyTag>;
using Density   = StrongReal<DensityTag>;
using Velocity  = StrongReal<VelocityTag>;
using Pressure  = StrongReal<PressureTag>;
using Stress    = StrongReal<StressTag>;
using Inventory = StrongReal<InventoryTag>;
using Latency   = StrongReal<LatencyTag>;
using Time      = StrongReal<TimeTag>;

}  // namespace continuum_lob
