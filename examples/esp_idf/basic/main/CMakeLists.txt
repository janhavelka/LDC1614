idf_component_register(
  SRCS
    "main.cpp"
    "Ldc1614IdfI2cTransport.cpp"
    "Ldc1614IdfCli.cpp"
  INCLUDE_DIRS "."
  REQUIRES LDC1614 esp_driver_i2c esp_driver_gpio esp_timer freertos
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

# The HIL transcript must identify the flashed target independently of the host
# checkout running the parser. A source archive without Git remains explicit
# "unknown" and cannot produce release-acceptance evidence.
set(LDC1614_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../..")
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${LDC1614_REPO_ROOT}"
  OUTPUT_VARIABLE LDC1614_EXAMPLE_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE LDC1614_GIT_RESULT
)
if(NOT LDC1614_GIT_RESULT EQUAL 0 OR LDC1614_EXAMPLE_GIT_COMMIT STREQUAL "")
  set(LDC1614_EXAMPLE_GIT_COMMIT "unknown")
endif()
execute_process(
  COMMAND git status --porcelain --untracked-files=all
  WORKING_DIRECTORY "${LDC1614_REPO_ROOT}"
  OUTPUT_VARIABLE LDC1614_EXAMPLE_GIT_CHANGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE LDC1614_GIT_STATUS_RESULT
)
if(NOT LDC1614_GIT_STATUS_RESULT EQUAL 0)
  set(LDC1614_EXAMPLE_GIT_STATUS "unknown")
elseif(LDC1614_EXAMPLE_GIT_CHANGES STREQUAL "")
  set(LDC1614_EXAMPLE_GIT_STATUS "clean")
else()
  set(LDC1614_EXAMPLE_GIT_STATUS "dirty")
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE
  "LDC1614_GIT_COMMIT=\"${LDC1614_EXAMPLE_GIT_COMMIT}\""
  "LDC1614_GIT_STATUS=\"${LDC1614_EXAMPLE_GIT_STATUS}\""
)
