idf_component_register(
  SRCS
    "main.cpp"
    "Ldc1614IdfI2cTransport.cpp"
    "../../../common/Ldc1614Cli.cpp"
  INCLUDE_DIRS "." "../../../.."
  REQUIRES LDC1614 esp_driver_i2c esp_driver_gpio esp_timer freertos
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
