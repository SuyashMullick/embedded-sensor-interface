cmake_minimum_required(VERSION 3.20.0)

# Zephyr expects the project to find the Zephyr package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(embedded_sensor_interface)

# Add source files
target_sources(app PRIVATE
    src/main.c
    src/uart_protocol.c
    src/sensor_sim.c
    src/parameters.c
    src/state_machine.c
)
