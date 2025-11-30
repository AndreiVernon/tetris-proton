cmake_minimum_required(VERSION 3.13)

# Initialize PIO hardware generation
pico_generate_pio_header(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)

# Add the source file
add_library(display_driver display.c)

# Link required libraries
target_link_libraries(display_driver 
    pico_stdlib 
    hardware_pio 
    hardware_dma
)