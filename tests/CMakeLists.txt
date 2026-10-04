add_executable(test_fields unit/test_fields.cpp)
target_link_libraries(test_fields PRIVATE continuum_lob::core)
add_test(NAME test_fields COMMAND test_fields)

add_executable(test_conservation numerical/test_conservation.cpp)
target_link_libraries(test_conservation PRIVATE continuum_lob::core)
add_test(NAME test_conservation COMMAND test_conservation)

add_executable(test_fv_solver numerical/test_fv_solver.cpp)
target_link_libraries(test_fv_solver PRIVATE continuum_lob::core)
add_test(NAME test_fv_solver COMMAND test_fv_solver)

add_executable(test_discrete discrete_lob/test_discrete.cpp)
target_link_libraries(test_discrete PRIVATE continuum_lob::core)
add_test(NAME test_discrete COMMAND test_discrete)

add_executable(test_fno neural_operator/test_fno.cpp)
target_link_libraries(test_fno PRIVATE continuum_lob::core)
add_test(NAME test_fno COMMAND test_fno)

add_executable(test_end_to_end end_to_end/test_pipeline.cpp)
target_link_libraries(test_end_to_end PRIVATE continuum_lob::core)
add_test(NAME test_end_to_end COMMAND test_end_to_end)
