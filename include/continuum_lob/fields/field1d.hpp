#pragma once
/// \file field1d.hpp
/// One-dimensional continuum field on a uniform mesh.
/// Author: Nabil Khondaker

#include "continuum_lob/core/types.hpp"
#include "continuum_lob/core/exceptions.hpp"
#include <vector>
#include <span>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace continuum_lob {

/// Uniform 1-D mesh on [x_min, x_max] with n_cells cells (n_cells+1 faces).
struct Mesh1D {
  Real x_min{-1.0};
  Real x_max{ 1.0};
  Index n_cells{64};

  Real dx() const {
    if (n_cells <= 0) throw NumericalError("n_cells must be positive");
    return (x_max - x_min) / static_cast<Real>(n_cells);
  }

  Real cell_center(Index i) const {
    return x_min + (static_cast<Real>(i) + 0.5) * dx();
  }

  Real face_position(Index i) const {
    return x_min + static_cast<Real>(i) * dx();
  }

  Index n_faces() const { return n_cells + 1; }
};

/// Cell-centered 1-D field.
class Field1D {
public:
  Field1D() = default;
  explicit Field1D(Mesh1D mesh, Real fill = 0.0);

  const Mesh1D& mesh() const { return mesh_; }
  Index size() const { return static_cast<Index>(data_.size()); }

  Real& operator[](Index i) { return data_[static_cast<Size>(i)]; }
  const Real& operator[](Index i) const { return data_[static_cast<Size>(i)]; }

  std::span<Real> data() { return data_; }
  std::span<const Real> data() const { return data_; }

  void fill(Real v);
  void zero();

  Real min() const;
  Real max() const;
  Real sum() const;
  Real l1_norm() const;
  Real l2_norm() const;
  Real linf_norm() const;

  /// Integral approximation: sum(u_i) * dx
  Real integral() const;

  Field1D& operator+=(const Field1D& o);
  Field1D& operator-=(const Field1D& o);
  Field1D& operator*=(Real s);

  friend Field1D operator+(Field1D a, const Field1D& b);
  friend Field1D operator-(Field1D a, const Field1D& b);
  friend Field1D operator*(Field1D a, Real s);

private:
  Mesh1D mesh_;
  std::vector<Real> data_;
};

/// Relative L2 error between two fields (same mesh).
Real relative_l2(const Field1D& a, const Field1D& b);

}  // namespace continuum_lob
