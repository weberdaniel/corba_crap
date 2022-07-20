cmake_minimum_required(VERSION 3.14)

project(external.omniorb)

set(VERSION 4.1.4)
set(DESCRIPTION "OmniORB")

include(ExternalProject)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

message("-- Creating Target OmniOrb")
ExternalProject_Add(
  omniorb URL ${CMAKE_CURRENT_LIST_DIR}/omniORB-4.1.4
  CONFIGURE_COMMAND mkdir <SOURCE_DIR>/build && cd <SOURCE_DIR>/build && <SOURCE_DIR>/configure --build-dir=<BINARY_DIR>
  INSTALL_COMMAND cmake -E echo "-- Skip install step.")
message("-- Creating Target OmniOrb - done")
