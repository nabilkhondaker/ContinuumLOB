#pragma once
#include "continuum_lob/core/types.hpp"
#include <cstdint>

namespace continuum_lob {

struct Order {
  std::uint64_t id{0};
  Side side{Side::Bid};
  Real price{0.0};
  Real quantity{0.0};
  Real timestamp{0.0};
  EventType type{EventType::LimitOrder};
  Index queue_position{0};
};

struct MarketEvent {
  Real timestamp{0.0};
  Side side{Side::Bid};
  Real price{0.0};
  Real quantity{0.0};
  EventType type{EventType::LimitOrder};
  std::uint64_t order_id{0};
  Index queue_position{0};
};

}  // namespace continuum_lob
