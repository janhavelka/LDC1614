find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/scripts/generate_version.py" check
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  RESULT_VARIABLE LDC1614_VERSION_RESULT
  OUTPUT_VARIABLE LDC1614_VERSION_OUTPUT
  ERROR_VARIABLE LDC1614_VERSION_ERROR
)

if(NOT LDC1614_VERSION_RESULT EQUAL 0)
  message(FATAL_ERROR "Generated version metadata (include/LDC1614/Version.h, idf_component.yml, Doxyfile) is missing or out of date; run: python scripts/generate_version.py sync\n${LDC1614_VERSION_OUTPUT} ${LDC1614_VERSION_ERROR}")
endif()

idf_component_register(
  SRCS "src/LDC1614.cpp"
  INCLUDE_DIRS "include"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
