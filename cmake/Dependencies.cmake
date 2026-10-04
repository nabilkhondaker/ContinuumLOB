# Continuum-LOB uses primarily the C++20 standard library.
# Optional third-party dependencies are detected when present.
# No mandatory external package managers are required for the core solver.

find_package(Threads REQUIRED)

# Optional: Eigen for linear algebra helpers (header-only if present)
find_path(EIGEN3_INCLUDE_DIR Eigen/Core
  HINTS /usr/include/eigen3 /usr/local/include/eigen3
)
if(EIGEN3_INCLUDE_DIR)
  message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIR}")
  set(CONTINUUM_LOB_HAS_EIGEN ON)
else()
  message(STATUS "Eigen3 not found; using built-in vector types")
  set(CONTINUUM_LOB_HAS_EIGEN OFF)
endif()
