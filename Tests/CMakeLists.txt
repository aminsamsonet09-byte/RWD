# Unit tests: platform-independent CPU engine only (no OpenFX, no Resolve).
add_executable(rwd_unit_tests
  unit/mini_test.h
  unit/test_main.cpp
  unit/test_engine.cpp
  unit/test_color.cpp)
target_link_libraries(rwd_unit_tests PRIVATE rwd_engine rwd_warnings)
add_test(NAME rwd_unit_tests COMMAND rwd_unit_tests)

# Plug-in load test: needs the built .ofx, so only when the plug-in is built.
if(RWD_BUILD_PLUGIN)
  add_executable(rwd_plugin_load_test plugin_load/plugin_load_test.cpp)
  target_include_directories(rwd_plugin_load_test SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/Plugin/third_party/openfx/include")
  if(NOT WIN32)
    target_link_libraries(rwd_plugin_load_test PRIVATE ${CMAKE_DL_LIBS})
  endif()
  add_dependencies(rwd_plugin_load_test RWD_COLOR_LAB)
  add_test(NAME rwd_plugin_load_test
           COMMAND rwd_plugin_load_test "$<TARGET_FILE:RWD_COLOR_LAB>")
endif()
