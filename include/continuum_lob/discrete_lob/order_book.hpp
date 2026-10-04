#pragma once
/// \file order_book.hpp
/// Event-driven discrete limit-order-book simulator.
/// Provides the high-fidelity reference against which continuum approximations
/// are validated.

#include "continuum_lob/core/types.hpp"
#include "continuum_lob/core/rng.hpp"
#include "continuum_lob/orders/order.hpp"
#include <map>
#include <vector>
#include <optional>
#include <cstdint>

namespace continuum_lob {

struct BookLevel {
  Real price{0.0};
  Real total_qty{0.0};
  Index n_orders{0};
};

struct BookSnapshot {
  Real mid{0.0};
  Real spread{0.0};
  Real best_bid{0.0};
  Real best_ask{0.0};
  std::vector<BookLevel> bids;
  std::vector<BookLevel> asks;
  Real time{0.0};
};

class DiscreteOrderBook {
public:
  explicit DiscreteOrderBook(Real tick_size = 0.01, Real initial_mid = 100.0);

  Real tick_size() const { return tick_; }
  Real mid() const;
  Real spread() const;
  Real best_bid() const;
  Real best_ask() const;

  /// Submit a limit order; returns order id.
  std::uint64_t add_limit(Side side, Real price, Real qty, Real ts);

  /// Cancel by order id.
  bool cancel(std::uint64_t order_id, Real ts);

  /// Aggressive market order; returns filled quantity and VWAP.
  struct FillResult {
    Real filled_qty{0.0};
    Real vwap{0.0};
    Real remaining{0.0};
  };
  FillResult market_order(Side side, Real qty, Real ts);

  BookSnapshot snapshot(Index depth = 10) const;

  /// Total volume on bid / ask within price distance of mid.
  Real volume_near_mid(Side side, Real distance) const;

  Index n_orders() const { return static_cast<Index>(orders_.size()); }

private:
  Real tick_;
  Real mid_ref_;
  std::uint64_t next_id_{1};
  // price -> (order_id -> quantity) for FIFO approximation
  std::map<Real, std::map<std::uint64_t, Real>, std::greater<Real>> bids_;
  std::map<Real, std::map<std::uint64_t, Real>> asks_;
  struct OrderMeta {
    Side side;
    Real price;
    Real qty;
  };
  std::map<std::uint64_t, OrderMeta> orders_;
};

}  // namespace continuum_lob
