cmake_minimum_required(VERSION 3.20)
project(continuum-lob VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CONTINUUM_LOB_BUILD_TESTS "Build tests" ON)

add_library(continuum_lob_core STATIC)
add_library(continuum_lob::core ALIAS continuum_lob_core)

target_include_directories(continuum_lob_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(continuum_lob_core PUBLIC cxx_std_20)

set(CORE_SOURCES
  src/core/types.cpp
  src/core/exceptions.cpp
  src/core/logging.cpp
  src/core/rng.cpp
  src/config/config.cpp
  src/units/strong_types.cpp
  src/time/time_grid.cpp
  src/events/market_event.cpp
  src/orders/order.cpp
  src/queue/queue_model.cpp
  src/discrete_lob/order_book.cpp
  src/discrete_lob/event_generator.cpp
  src/coarse_graining/kernel.cpp
  src/coarse_graining/projector.cpp
  src/fields/field1d.cpp
  src/fields/state.cpp
  src/density/density_field.cpp
  src/velocity/velocity_field.cpp
  src/pressure/pressure_field.cpp
  src/stress/stress_field.cpp
  src/constitutive/constitutive_model.cpp
  src/forcing/stochastic_forcing.cpp
  src/stochastic/noise.cpp
  src/pde/continuum_pde.cpp
  src/conservation/conservation.cpp
  src/boundary/boundary_conditions.cpp
  src/finite_volume/fv_solver.cpp
  src/reconstruction/muscl.cpp
  src/reconstruction/weno.cpp
  src/flux/rusanov.cpp
  src/flux/hll.cpp
  src/riemann/riemann_solver.cpp
  src/spectral/fft_ops.cpp
  src/timestep/cfl.cpp
  src/timestep/adaptive.cpp
  src/convergence/convergence.cpp
  src/verification/manufactured.cpp
  src/datasets/dataset_generator.cpp
  src/neural_operator/operator_base.cpp
  src/fno/spectral_conv.cpp
  src/fno/fno_model.cpp
  src/deeponet/deeponet.cpp
  src/physics_loss/physics_loss.cpp
  src/surrogate/surrogate.cpp
  src/latency/latency_model.cpp
  src/execution/execution_env.cpp
  src/market_making/mm_env.cpp
  src/impact/impact_model.cpp
  src/adverse_selection/adverse_selection.cpp
  src/inventory/inventory.cpp
  src/execution_baselines/baselines.cpp
  src/rl/mdp.cpp
  src/sac/sac_agent.cpp
  src/sac/replay_buffer.cpp
  src/environments/environment.cpp
  src/constraints/action_constraints.cpp
  src/risk/risk_metrics.cpp
  src/statistics/statistics.cpp
  src/reproducibility/seed_manager.cpp
  src/serialization/serialize.cpp
  src/reporting/report.cpp
  src/visualization/export_fields.cpp
  src/utilities/math_utils.cpp
)
target_sources(continuum_lob_core PRIVATE ${CORE_SOURCES})

add_executable(continuum-lob apps/continuum-lob/main.cpp)
target_link_libraries(continuum-lob PRIVATE continuum_lob::core)

add_executable(discrete-lob apps/discrete-lob/main.cpp)
target_link_libraries(discrete-lob PRIVATE continuum_lob::core)

add_executable(generate-dataset apps/generate-dataset/main.cpp)
target_link_libraries(generate-dataset PRIVATE continuum_lob::core)

add_executable(evaluate-surrogate apps/evaluate-surrogate/main.cpp)
target_link_libraries(evaluate-surrogate PRIVATE continuum_lob::core)

add_executable(run-execution apps/run-execution/main.cpp)
target_link_libraries(run-execution PRIVATE continuum_lob::core)

add_executable(benchmark apps/benchmark/main.cpp)
target_link_libraries(benchmark PRIVATE continuum_lob::core)

if(CONTINUUM_LOB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
