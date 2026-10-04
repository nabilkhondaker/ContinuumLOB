#include "continuum_lob/fields/field1d.hpp"
#include <numeric>
#include <cmath>
#include <algorithm>

namespace continuum_lob {

Field1D::Field1D(Mesh1D mesh, Real fill)
    : mesh_(std::move(mesh)),
      data_(static_cast<Size>(mesh_.n_cells), fill) {
  if (mesh_.n_cells <= 0) {
    throw NumericalError("Field1D: n_cells must be positive");
  }
}

void Field1D::fill(Real v) {
  std::fill(data_.begin(), data_.end(), v);
}

void Field1D::zero() { fill(0.0); }

Real Field1D::min() const {
  if (data_.empty()) return 0.0;
  return *std::min_element(data_.begin(), data_.end());
}

Real Field1D::max() const {
  if (data_.empty()) return 0.0;
  return *std::max_element(data_.begin(), data_.end());
}

Real Field1D::sum() const {
  return std::accumulate(data_.begin(), data_.end(), 0.0);
}

Real Field1D::l1_norm() const {
  Real s = 0.0;
  for (Real v : data_) s += std::abs(v);
  return s;
}

Real Field1D::l2_norm() const {
  Real s = 0.0;
  for (Real v : data_) s += v * v;
  return std::sqrt(s);
}

Real Field1D::linf_norm() const {
  Real m = 0.0;
  for (Real v : data_) m = std::max(m, std::abs(v));
  return m;
}

Real Field1D::integral() const {
  return sum() * mesh_.dx();
}

Field1D& Field1D::operator+=(const Field1D& o) {
  if (size() != o.size()) throw DimensionMismatch("Field1D += size mismatch");
  for (Size i = 0; i < data_.size(); ++i) data_[i] += o.data_[i];
  return *this;
}

Field1D& Field1D::operator-=(const Field1D& o) {
  if (size() != o.size()) throw DimensionMismatch("Field1D -= size mismatch");
  for (Size i = 0; i < data_.size(); ++i) data_[i] -= o.data_[i];
  return *this;
}

Field1D& Field1D::operator*=(Real s) {
  for (Real& v : data_) v *= s;
  return *this;
}

Field1D operator+(Field1D a, const Field1D& b) {
  a += b;
  return a;
}

Field1D operator-(Field1D a, const Field1D& b) {
  a -= b;
  return a;
}

Field1D operator*(Field1D a, Real s) {
  a *= s;
  return a;
}

Real relative_l2(const Field1D& a, const Field1D& b) {
  if (a.size() != b.size()) throw DimensionMismatch("relative_l2 size mismatch");
  Real num = 0.0, den = 0.0;
  for (Index i = 0; i < a.size(); ++i) {
    Real d = a[i] - b[i];
    num += d * d;
    den += b[i] * b[i];
  }
  if (den < 1e-30) return std::sqrt(num);
  return std::sqrt(num / den);
}

}  // namespace continuum_lob
