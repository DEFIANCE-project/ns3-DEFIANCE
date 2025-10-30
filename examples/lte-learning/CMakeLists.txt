find_package(Python COMPONENTS Interpreter Development)
if(NOT Python_FOUND)
  message(
    ${HIGHLIGHTED_STATUS}
    "lte-learning disabled: Python not found"
  )
  return()
endif()

find_package(pybind11 CONFIG)
if(NOT pybind11_FOUND)
    message(
      ${HIGHLIGHTED_STATUS}
      "lte-learning disabled: pybind11 not found"
    )
    return()
endif()
include_directories(${pybind11_INCLUDE_DIRS})

build_lib_example(
        NAME defiance-lte-learning
        SOURCE_FILES
                lte-learning/pc-simulation.cc
                lte-learning/pc-environment.cc
                lte-learning/topology-creator.cc
                lte-learning/sumo-environment.cc
                lte-learning/static-environment.cc
                lte-learning/device-manager.cc
                lte-learning/base-environment.cc
        HEADER_FILES
                lte-learning/pc-environment.h
                lte-learning/topology-creator.h
                lte-learning/sumo-environment.h
                lte-learning/static-environment.h
                lte-learning/device-manager.h
                lte-learning/base-environment.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
                ${Python_LIBRARIES}
)
