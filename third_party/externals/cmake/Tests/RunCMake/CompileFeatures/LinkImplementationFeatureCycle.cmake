
add_library(empty1 empty.cpp)

add_library(empty2 INTERFACE)
add_library(empty3 INTERFACE)
target_compile_features(empty3 INTERFACE cxx_static_assert)

target_link_libraries(empty1
  # When starting, $<COMPILE_FEATURES:cxx_auto_type> is '0', so 'freeze' the
  # CXX_STANDARD at 98 during computation.
  $<$<COMPILE_FEATURES:cxx_auto_type>:empty2>
  # This would add cxx_static_assert, but that would require CXX_STANDARD = 11,
  # which is not allowed after freeze.  Report an error.
  empty3
)
