cmake_minimum_required(VERSION 3.17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
aux_source_directory(./ DIR_SRCS)
add_executable(demo_4 ${DIR_SRCS})