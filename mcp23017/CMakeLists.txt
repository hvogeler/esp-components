idf_component_register(SRCS "mcp23017.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES driver i2c espressif__esp-idf-cxx)
