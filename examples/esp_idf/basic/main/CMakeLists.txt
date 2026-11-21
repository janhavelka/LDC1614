idf_component_register(
  SRCS
    "main.cpp"
    "../../../esp32/I2cMasterTransport.cpp"
    "Ldc161xIdfCli.cpp"
  INCLUDE_DIRS "." "../../../esp32"
  REQUIRES LDC161x esp_driver_i2c esp_driver_gpio esp_timer freertos
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

# The HIL transcript must identify the flashed target independently of the host
# checkout running the parser. A source archive without Git remains explicit
# "unknown" and cannot produce release-acceptance evidence.
set(LDC161X_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../..")
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${LDC161X_REPO_ROOT}"
  OUTPUT_VARIABLE LDC161X_EXAMPLE_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE LDC161X_GIT_RESULT
)
if(NOT LDC161X_GIT_RESULT EQUAL 0 OR LDC161X_EXAMPLE_GIT_COMMIT STREQUAL "")
  set(LDC161X_EXAMPLE_GIT_COMMIT "unknown")
endif()
string(TIMESTAMP LDC161X_EXAMPLE_BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%S" UTC)
string(SUBSTRING "${LDC161X_EXAMPLE_BUILD_TIMESTAMP}" 0 10
       LDC161X_EXAMPLE_BUILD_DATE)
string(SUBSTRING "${LDC161X_EXAMPLE_BUILD_TIMESTAMP}" 11 8
       LDC161X_EXAMPLE_BUILD_TIME)
execute_process(
  COMMAND git status --porcelain --untracked-files=all
  WORKING_DIRECTORY "${LDC161X_REPO_ROOT}"
  OUTPUT_VARIABLE LDC161X_EXAMPLE_GIT_CHANGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE LDC161X_GIT_STATUS_RESULT
)
if(NOT LDC161X_GIT_STATUS_RESULT EQUAL 0)
  set(LDC161X_EXAMPLE_GIT_STATUS "unknown")
elseif(LDC161X_EXAMPLE_GIT_CHANGES STREQUAL "")
  set(LDC161X_EXAMPLE_GIT_STATUS "clean")
else()
  set(LDC161X_EXAMPLE_GIT_STATUS "dirty")
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE
  "LDC161X_GIT_COMMIT=\"${LDC161X_EXAMPLE_GIT_COMMIT}\""
  "LDC161X_GIT_STATUS=\"${LDC161X_EXAMPLE_GIT_STATUS}\""
  "LDC161X_BUILD_DATE=\"${LDC161X_EXAMPLE_BUILD_DATE}\""
  "LDC161X_BUILD_TIME=\"${LDC161X_EXAMPLE_BUILD_TIME}\""
)
