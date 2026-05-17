cmake_minimum_required(VERSION 3.15)
project(ccc_sm)

add_library(ccc_sm SHARED
  ccc_sm_agent.c
  ccc_sm_ric.c
)
target_compile_definitions(ccc_sm PUBLIC ${E2AP_VERSION} ${KPM_VERSION})
target_compile_options(ccc_sm PRIVATE -fvisibility=hidden -Wno-unused-parameter)

add_library(ccc_sm_static STATIC
  ccc_sm_agent.c
  ccc_sm_ric.c
)
target_compile_definitions(ccc_sm_static PUBLIC ${E2AP_VERSION} ${KPM_VERSION})
target_compile_options(ccc_sm_static PRIVATE -Wno-unused-parameter)
