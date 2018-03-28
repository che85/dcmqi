cmake_minimum_required(VERSION 3.5.0)

#-----------------------------------------------------------------------------

#
# DCMQI
#
if(NOT DCMQI_SOURCE_DIR AND NOT Slicer_SOURCE_DIR)
  find_package(DCMQI REQUIRED)
endif()

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
set(MODULE_NAME tid1500writer)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES dcmqi
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
set(MODULE_NAME tid1500reader)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES dcmqi
  EXECUTABLE_ONLY
  )

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dcmtkplus)
SEMMacroBuildCLI(
  NAME qiicrxSR
  TARGET_LIBRARIES dcmqi
  #ADDITIONAL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dcmtkplus/tidqiicrx.cc
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
