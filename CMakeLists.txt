cmake_minimum_required(VERSION 3.12)
project(Enhanced_Local_Tone_Mapping_TMO)
file(GLOB_RECURSE ELTM_SRC "ELTM TMO/*.cpp")
add_executable(ELTM-TMO ${ELTM_SRC})