function(continuum_lob_set_project_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wconversion -Wsign-conversion
      -Wshadow -Wnon-virtual-dtor
      -Wold-style-cast -Wcast-align
      -Wunused -Woverloaded-virtual
      -Wnull-dereference
      -Wno-unused-parameter
    )
  elseif(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  endif()
endfunction()
