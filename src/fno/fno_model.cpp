#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/core/exceptions.hpp"
#include <numeric>
#include <algorithm>

namespace continuum_lob {

namespace {

void dft(const std::vector<Real>& in, std::vector<std::complex<Real>>& out) {
  const Index N = static_cast<Index>(in.size());
  out.resize(static_cast<Size>(N));
  for (Index k = 0; k < N; ++k) {
    std::complex<Real> s{0.0, 0.0};
    for (Index n = 0; n < N; ++n) {
      Real angle = -2.0 * kPi * static_cast<Real>(k * n) / static_cast<Real>(N);
      s += in[static_cast<Size>(n)] * std::complex<Real>{std::cos(angle), std::sin(angle)};
    }
    out[static_cast<Size>(k)] = s;
  }
}

void idft(const std::vector<std::complex<Real>>& in, std::vector<Real>& out) {
  const Index N = static_cast<Index>(in.size());
  out.resize(static_cast<Size>(N));
  for (Index n = 0; n < N; ++n) {
    std::complex<Real> s{0.0, 0.0};
    for (Index k = 0; k < N; ++k) {
      Real angle = 2.0 * kPi * static_cast<Real>(k * n) / static_cast<Real>(N);
      s += in[static_cast<Size>(k)] * std::complex<Real>{std::cos(angle), std::sin(angle)};
    }
    out[static_cast<Size>(n)] = s.real() / static_cast<Real>(N);
  }
}

}  // namespace

SpectralConv1D::SpectralConv1D(Index n_modes, Index in_ch, Index out_ch, RngStream& rng)
    : n_modes_(n_modes), in_ch_(in_ch), out_ch_(out_ch) {
  weights.resize(static_cast<Size>(out_ch));
  for (Index oc = 0; oc < out_ch; ++oc) {
    weights[static_cast<Size>(oc)].resize(static_cast<Size>(in_ch));
    for (Index ic = 0; ic < in_ch; ++ic) {
      weights[static_cast<Size>(oc)][static_cast<Size>(ic)].resize(static_cast<Size>(n_modes));
      for (Index m = 0; m < n_modes; ++m) {
        Real re = rng.normal(0.0, 0.02);
        Real im = rng.normal(0.0, 0.02);
        weights[static_cast<Size>(oc)][static_cast<Size>(ic)][static_cast<Size>(m)] = {re, im};
      }
    }
  }
}

void SpectralConv1D::forward(const std::vector<std::vector<Real>>& in,
                             std::vector<std::vector<Real>>& out) const {
  const Index in_ch = static_cast<Index>(in.size());
  if (in_ch != in_ch_) throw DimensionMismatch("SpectralConv1D channel mismatch");
  const Index N = static_cast<Index>(in[0].size());
  out.assign(static_cast<Size>(out_ch_), std::vector<Real>(static_cast<Size>(N), 0.0));

  std::vector<std::vector<std::complex<Real>>> in_hat(static_cast<Size>(in_ch_));
  for (Index ic = 0; ic < in_ch_; ++ic) {
    dft(in[static_cast<Size>(ic)], in_hat[static_cast<Size>(ic)]);
  }

  Index modes = std::min(n_modes_, N / 2 + 1);
  for (Index oc = 0; oc < out_ch_; ++oc) {
    std::vector<std::complex<Real>> out_hat(static_cast<Size>(N), {0.0, 0.0});
    for (Index ic = 0; ic < in_ch_; ++ic) {
      for (Index m = 0; m < modes; ++m) {
        out_hat[static_cast<Size>(m)] +=
            weights[static_cast<Size>(oc)][static_cast<Size>(ic)][static_cast<Size>(m)] *
            in_hat[static_cast<Size>(ic)][static_cast<Size>(m)];
        if (m > 0 && m < N - m) {
          Index mn = N - m;
          out_hat[static_cast<Size>(mn)] +=
              std::conj(weights[static_cast<Size>(oc)][static_cast<Size>(ic)][static_cast<Size>(m)]) *
              in_hat[static_cast<Size>(ic)][static_cast<Size>(mn)];
        }
      }
    }
    idft(out_hat, out[static_cast<Size>(oc)]);
  }
}

FNOModel::FNOModel(FNOConfig cfg, RngStream& rng) : cfg_(std::move(cfg)) {
  lift_w_.assign(static_cast<Size>(cfg_.width),
                 std::vector<Real>(static_cast<Size>(cfg_.n_input_channels)));
  lift_b_.assign(static_cast<Size>(cfg_.width), 0.0);
  for (Index o = 0; o < cfg_.width; ++o) {
    for (Index i = 0; i < cfg_.n_input_channels; ++i) {
      lift_w_[static_cast<Size>(o)][static_cast<Size>(i)] = rng.normal(0.0, 0.1);
    }
    lift_b_[static_cast<Size>(o)] = rng.normal(0.0, 0.01);
  }

  spectral_.reserve(static_cast<Size>(cfg_.n_layers));
  pw_w_.resize(static_cast<Size>(cfg_.n_layers));
  for (Index l = 0; l < cfg_.n_layers; ++l) {
    spectral_.emplace_back(cfg_.n_modes, cfg_.width, cfg_.width, rng);
    pw_w_[static_cast<Size>(l)].assign(static_cast<Size>(cfg_.width),
                                       std::vector<Real>(static_cast<Size>(cfg_.width)));
    for (Index o = 0; o < cfg_.width; ++o)
      for (Index i = 0; i < cfg_.width; ++i)
        pw_w_[static_cast<Size>(l)][static_cast<Size>(o)][static_cast<Size>(i)] =
            rng.normal(0.0, 0.05);
  }

  proj_w_.assign(static_cast<Size>(cfg_.n_output_channels),
                 std::vector<Real>(static_cast<Size>(cfg_.width)));
  proj_b_.assign(static_cast<Size>(cfg_.n_output_channels), 0.0);
  for (Index o = 0; o < cfg_.n_output_channels; ++o) {
    for (Index i = 0; i < cfg_.width; ++i)
      proj_w_[static_cast<Size>(o)][static_cast<Size>(i)] = rng.normal(0.0, 0.1);
  }
}

void FNOModel::forward(const std::vector<Field1D>& in_fields,
                       std::vector<Field1D>& out_fields) const {
  if (static_cast<Index>(in_fields.size()) != cfg_.n_input_channels) {
    throw DimensionMismatch("FNO input channel count");
  }
  const Index N = in_fields[0].size();
  Mesh1D mesh = in_fields[0].mesh();

  std::vector<std::vector<Real>> z(static_cast<Size>(cfg_.width),
                                   std::vector<Real>(static_cast<Size>(N), 0.0));
  for (Index c = 0; c < cfg_.width; ++c) {
    for (Index i = 0; i < N; ++i) {
      Real s = lift_b_[static_cast<Size>(c)];
      for (Index ic = 0; ic < cfg_.n_input_channels; ++ic) {
        s += lift_w_[static_cast<Size>(c)][static_cast<Size>(ic)] * in_fields[static_cast<Size>(ic)][i];
      }
      z[static_cast<Size>(c)][static_cast<Size>(i)] = s;
    }
  }

  for (Index l = 0; l < cfg_.n_layers; ++l) {
    std::vector<std::vector<Real>> spect_out;
    spectral_[static_cast<Size>(l)].forward(z, spect_out);

    std::vector<std::vector<Real>> z_new(static_cast<Size>(cfg_.width),
                                         std::vector<Real>(static_cast<Size>(N), 0.0));
    for (Index c = 0; c < cfg_.width; ++c) {
      for (Index i = 0; i < N; ++i) {
        Real pw = 0.0;
        for (Index k = 0; k < cfg_.width; ++k) {
          pw += pw_w_[static_cast<Size>(l)][static_cast<Size>(c)][static_cast<Size>(k)] *
                z[static_cast<Size>(k)][static_cast<Size>(i)];
        }
        Real v = spect_out[static_cast<Size>(c)][static_cast<Size>(i)] + pw;
        z_new[static_cast<Size>(c)][static_cast<Size>(i)] = gelu(v);
      }
    }
    z = std::move(z_new);
  }

  out_fields.resize(static_cast<Size>(cfg_.n_output_channels));
  for (Index oc = 0; oc < cfg_.n_output_channels; ++oc) {
    out_fields[static_cast<Size>(oc)] = Field1D(mesh, 0.0);
    for (Index i = 0; i < N; ++i) {
      Real s = proj_b_[static_cast<Size>(oc)];
      for (Index c = 0; c < cfg_.width; ++c) {
        s += proj_w_[static_cast<Size>(oc)][static_cast<Size>(c)] * z[static_cast<Size>(c)][static_cast<Size>(i)];
      }
      out_fields[static_cast<Size>(oc)][i] = s;
    }
  }
}

void FNOModel::predict(const Field1D& rho, const Field1D& u,
                       Field1D& rho_pred, Field1D& u_pred) const {
  std::vector<Field1D> in{rho, u};
  std::vector<Field1D> out;
  forward(in, out);
  if (out.size() >= 2) {
    rho_pred = out[0];
    u_pred = out[1];
  }
}

Index FNOModel::parameter_count() const {
  Index n = 0;
  n += cfg_.width * cfg_.n_input_channels + cfg_.width;
  for (const auto& sc : spectral_) {
    n += sc.out_channels() * sc.in_channels() * sc.n_modes() * 2;
  }
  n += cfg_.n_layers * cfg_.width * cfg_.width;
  n += cfg_.n_output_channels * cfg_.width + cfg_.n_output_channels;
  return n;
}

Real FNOModel::mse_loss(const std::vector<Field1D>& pred,
                        const std::vector<Field1D>& target) const {
  if (pred.size() != target.size()) throw DimensionMismatch("mse_loss");
  Real s = 0.0;
  Index count = 0;
  for (Size c = 0; c < pred.size(); ++c) {
    for (Index i = 0; i < pred[c].size(); ++i) {
      Real d = pred[c][i] - target[c][i];
      s += d * d;
      ++count;
    }
  }
  return (count > 0) ? s / static_cast<Real>(count) : 0.0;
}

}  // namespace continuum_lob
