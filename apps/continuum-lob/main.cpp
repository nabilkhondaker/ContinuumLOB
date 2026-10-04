/// Continuum-LOB main CLI entry point.
/// Author: Nabil Khondaker

#include "continuum_lob/fields/state.hpp"
#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/constitutive/constitutive_model.hpp"
#include "continuum_lob/discrete_lob/order_book.hpp"
#include "continuum_lob/coarse_graining/projector.hpp"
#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/execution/execution_env.hpp"
#include "continuum_lob/core/rng.hpp"
#include "continuum_lob/core/exceptions.hpp"

#include <iostream>
#include <string>
#include <cmath>

using namespace continuum_lob;

namespace {
void print_help() {
  std::cout <<
"Continuum-LOB — Physics-informed neural operators for order-book continuum models\n"
"\n"
"Usage:\n"
"  continuum-lob <command> [options]\n"
"\n"
"Commands:\n"
"  help                 Show this help\n"
"  simulate             Run a tiny continuum finite-volume simulation\n"
"  discrete             Run a discrete LOB vignette and coarse-grain it\n"
"  fno-demo             Instantiate FNO and run a forward pass\n"
"  execution            Run a TWAP vs market execution episode\n"
"  version              Print version\n"
"\n"
"This is a research framework. Results are synthetic.\n";
}

void cmd_simulate() {
  std::cout << "=== Continuum finite-volume simulation ===\n";
  Mesh1D mesh;
  mesh.x_min = -1.0;
  mesh.x_max = 1.0;
  mesh.n_cells = 64;

  ContinuumState state(mesh);
  for (Index i = 0; i < mesh.n_cells; ++i) {
    Real x = mesh.cell_center(i);
    state.rho[i] = 1.0 + 0.5 * std::exp(-x * x / 0.05);
    state.u[i] = 0.1 * std::sin(kPi * x);
  }

  PDEParams pde;
  pde.constitutive.pressure_model = PressureModel::Linear;
  pde.constitutive.c = 1.0;
  pde.constitutive.mu0 = 0.02;
  pde.bc = BCType::ZeroFlux;

  SolverConfig scfg;
  scfg.cfl = 0.3;
  scfg.t_final = 0.2;
  scfg.dt_max = 0.005;
  scfg.adaptive_dt = true;
  scfg.use_muscl = true;

  FiniteVolumeSolver solver(pde, scfg);
  Real mass0 = solver.pde().total_mass(state.rho);
  std::cout << "Initial mass: " << mass0 << "\n";
  std::cout << "Initial max density: " << state.rho.max() << "\n";

  auto diag = solver.run(state, nullptr);
  std::cout << "Steps: " << diag.steps << "\n";
  std::cout << "Final time: " << diag.final_time << "\n";
  std::cout << "Final mass: " << diag.mass_final << "\n";
  std::cout << "Mass error |M_f - M_0|: " << diag.mass_error << "\n";
  std::cout << "Max wave speed: " << diag.max_wave_speed << "\n";
  std::cout << "dt range: [" << diag.min_dt << ", " << diag.max_dt << "]\n";
  std::cout << "Final max density: " << state.rho.max() << "\n";
  std::cout << "OK\n";
}

void cmd_discrete() {
  std::cout << "=== Discrete LOB + coarse-graining ===\n";
  DiscreteOrderBook book(0.01, 100.0);
  for (int i = 0; i < 10; ++i) {
    book.add_limit(Side::Bid, 100.0 - 0.01 * (i + 1), 50.0 + 10.0 * i, 0.0);
    book.add_limit(Side::Ask, 100.0 + 0.01 * (i + 1), 50.0 + 10.0 * i, 0.0);
  }
  auto snap = book.snapshot(10);
  std::cout << "Mid: " << snap.mid << "  Spread: " << snap.spread << "\n";
  std::cout << "Best bid: " << snap.best_bid << "  Best ask: " << snap.best_ask << "\n";

  ContinuumProjector proj;
  auto state = proj.project(snap, snap.mid);
  std::cout << "Continuum cells: " << state.rho.size() << "\n";
  std::cout << "Projected total mass: " << state.rho.integral() << "\n";
  std::cout << "Bid mass: " << state.rho_bid.integral()
            << "  Ask mass: " << state.rho_ask.integral() << "\n";
  std::cout << "OK\n";
}

void cmd_fno_demo() {
  std::cout << "=== FNO forward pass demo ===\n";
  RngStream rng(123);
  FNOConfig cfg;
  cfg.n_modes = 8;
  cfg.width = 16;
  cfg.n_layers = 2;
  cfg.n_input_channels = 2;
  cfg.n_output_channels = 2;

  FNOModel model(cfg, rng);
  std::cout << "Parameter count: " << model.parameter_count() << "\n";

  Mesh1D mesh;
  mesh.n_cells = 32;
  Field1D rho(mesh, 1.0), u(mesh, 0.0);
  for (Index i = 0; i < mesh.n_cells; ++i) {
    Real x = mesh.cell_center(i);
    rho[i] = 1.0 + 0.2 * std::sin(2.0 * kPi * x);
    u[i] = 0.05 * std::cos(2.0 * kPi * x);
  }
  Field1D rho_p(mesh), u_p(mesh);
  model.predict(rho, u, rho_p, u_p);
  std::cout << "Input rho L2: " << rho.l2_norm() << "\n";
  std::cout << "Pred  rho L2: " << rho_p.l2_norm() << "\n";
  std::cout << "OK (untrained weights; forward pass only)\n";
}

void cmd_execution() {
  std::cout << "=== Execution episode (TWAP vs market) ===\n";
  RngStream rng(7);
  ExecutionConfig ecfg;
  ecfg.target_qty = 50.0;
  ecfg.horizon = 0.5;
  ecfg.n_steps = 10;
  ecfg.arrival_price = 100.0;

  PDEParams pde;
  SolverConfig scfg;
  scfg.t_final = 0.05;
  scfg.adaptive_dt = false;
  scfg.dt_max = 0.01;

  {
    ExecutionEnvironment env(ecfg, pde, scfg);
    env.reset(rng);
    Real total_reward = 0.0;
    Real total_fill = 0.0;
    for (Index k = 0; k < ecfg.n_steps; ++k) {
      auto a = ExecutionEnvironment::twap_action(env.state(), ecfg);
      auto r = env.step(a, rng);
      total_reward += r.reward;
      total_fill += r.fill_qty;
      if (r.done) break;
    }
    std::cout << "TWAP:   reward=" << total_reward
              << "  filled=" << total_fill
              << "  remaining=" << env.state().remaining << "\n";
  }

  {
    RngStream rng2(7);
    ExecutionEnvironment env(ecfg, pde, scfg);
    env.reset(rng2);
    auto a = ExecutionEnvironment::market_all(env.state());
    auto r = env.step(a, rng2);
    std::cout << "Market: reward=" << r.reward
              << "  filled=" << r.fill_qty
              << "  remaining=" << env.state().remaining << "\n";
  }
  std::cout << "OK\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      print_help();
      return 0;
    }
    std::string cmd = argv[1];
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
      print_help();
    } else if (cmd == "version") {
      std::cout << "continuum-lob 0.1.0\n";
    } else if (cmd == "simulate") {
      cmd_simulate();
    } else if (cmd == "discrete") {
      cmd_discrete();
    } else if (cmd == "fno-demo") {
      cmd_fno_demo();
    } else if (cmd == "execution") {
      cmd_execution();
    } else {
      std::cerr << "Unknown command: " << cmd << "\n";
      print_help();
      return 1;
    }
  } catch (const ContinuumLobError& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 2;
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 3;
  }
  return 0;
}
