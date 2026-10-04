#include "continuum_lob/discrete_lob/order_book.hpp"
#include <algorithm>
#include <cmath>

namespace continuum_lob {

DiscreteOrderBook::DiscreteOrderBook(Real tick_size, Real initial_mid)
    : tick_(tick_size), mid_ref_(initial_mid) {}

Real DiscreteOrderBook::best_bid() const {
  if (bids_.empty()) return mid_ref_ - 10.0 * tick_;
  return bids_.begin()->first;
}

Real DiscreteOrderBook::best_ask() const {
  if (asks_.empty()) return mid_ref_ + 10.0 * tick_;
  return asks_.begin()->first;
}

Real DiscreteOrderBook::mid() const {
  return 0.5 * (best_bid() + best_ask());
}

Real DiscreteOrderBook::spread() const {
  return best_ask() - best_bid();
}

std::uint64_t DiscreteOrderBook::add_limit(Side side, Real price, Real qty, Real /*ts*/) {
  // Snap to tick
  price = std::round(price / tick_) * tick_;
  auto id = next_id_++;
  if (side == Side::Bid) {
    bids_[price][id] = qty;
  } else {
    asks_[price][id] = qty;
  }
  orders_[id] = OrderMeta{side, price, qty};
  return id;
}

bool DiscreteOrderBook::cancel(std::uint64_t order_id, Real /*ts*/) {
  auto it = orders_.find(order_id);
  if (it == orders_.end()) return false;
  const auto& meta = it->second;
  if (meta.side == Side::Bid) {
    auto pit = bids_.find(meta.price);
    if (pit != bids_.end()) {
      pit->second.erase(order_id);
      if (pit->second.empty()) bids_.erase(pit);
    }
  } else {
    auto pit = asks_.find(meta.price);
    if (pit != asks_.end()) {
      pit->second.erase(order_id);
      if (pit->second.empty()) asks_.erase(pit);
    }
  }
  orders_.erase(it);
  return true;
}

DiscreteOrderBook::FillResult DiscreteOrderBook::market_order(Side side, Real qty, Real /*ts*/) {
  FillResult res;
  res.remaining = qty;
  Real notional = 0.0;

  if (side == Side::Bid) {
    // Buy: lift asks
    while (res.remaining > 1e-12 && !asks_.empty()) {
      auto& level = asks_.begin()->second;
      while (res.remaining > 1e-12 && !level.empty()) {
        auto oit = level.begin();
        Real take = std::min(res.remaining, oit->second);
        notional += take * asks_.begin()->first;
        res.filled_qty += take;
        res.remaining -= take;
        oit->second -= take;
        if (oit->second < 1e-12) {
          orders_.erase(oit->first);
          level.erase(oit);
        }
      }
      if (level.empty()) asks_.erase(asks_.begin());
    }
  } else {
    // Sell: hit bids
    while (res.remaining > 1e-12 && !bids_.empty()) {
      auto& level = bids_.begin()->second;
      while (res.remaining > 1e-12 && !level.empty()) {
        auto oit = level.begin();
        Real take = std::min(res.remaining, oit->second);
        notional += take * bids_.begin()->first;
        res.filled_qty += take;
        res.remaining -= take;
        oit->second -= take;
        if (oit->second < 1e-12) {
          orders_.erase(oit->first);
          level.erase(oit);
        }
      }
      if (level.empty()) bids_.erase(bids_.begin());
    }
  }
  res.vwap = (res.filled_qty > 1e-12) ? notional / res.filled_qty : 0.0;
  return res;
}

BookSnapshot DiscreteOrderBook::snapshot(Index depth) const {
  BookSnapshot snap;
  snap.mid = mid();
  snap.spread = spread();
  snap.best_bid = best_bid();
  snap.best_ask = best_ask();
  Index c = 0;
  for (const auto& [px, level] : bids_) {
    if (c >= depth) break;
    Real q = 0.0;
    for (const auto& [id, qq] : level) q += qq;
    snap.bids.push_back(BookLevel{px, q, static_cast<Index>(level.size())});
    ++c;
  }
  c = 0;
  for (const auto& [px, level] : asks_) {
    if (c >= depth) break;
    Real q = 0.0;
    for (const auto& [id, qq] : level) q += qq;
    snap.asks.push_back(BookLevel{px, q, static_cast<Index>(level.size())});
    ++c;
  }
  return snap;
}

Real DiscreteOrderBook::volume_near_mid(Side side, Real distance) const {
  Real m = mid();
  Real vol = 0.0;
  if (side == Side::Bid) {
    for (const auto& [px, level] : bids_) {
      if (m - px > distance) break;
      for (const auto& [id, q] : level) vol += q;
    }
  } else {
    for (const auto& [px, level] : asks_) {
      if (px - m > distance) break;
      for (const auto& [id, q] : level) vol += q;
    }
  }
  return vol;
}

}  // namespace continuum_lob
