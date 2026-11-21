find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/scripts/generate_version.py" check
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  RESULT_VARIABLE LDC161X_VERSION_RESULT
  OUTPUT_VARIABLE LDC161X_VERSION_OUTPUT
  ERROR_VARIABLE LDC161X_VERSION_ERROR
)

if(NOT LDC161X_VERSION_RESULT EQUAL 0)
  message(FATAL_ERROR "Generated version metadata (include/LDC161x/Version.h, idf_component.yml, Doxyfile) is missing or out of date; run: python scripts/generate_version.py sync\n${LDC161X_VERSION_OUTPUT} ${LDC161X_VERSION_ERROR}")
endif()

idf_component_register(
  SRCS "src/LDC161x.cpp"
  INCLUDE_DIRS "include"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
