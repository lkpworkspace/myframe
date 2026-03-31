# UseSWIG generates standard target names.
if (POLICY CMP0078) # >= 3.13
  cmake_policy(SET CMP0078 NEW)
endif()
# Changed in version 3.14: If policy CMP0086 is set to NEW, -module <module_name> is passed to SWIG compiler.
if (POLICY CMP0086) # >= 3.14
  cmake_policy(SET CMP0086 NEW)
endif()
project(pymyframe)

include(${SWIG_USE_FILE})

set(dot_i_srcs
  swig/pymyframe.i
)
set_property(SOURCE ${dot_i_srcs} PROPERTY CPLUSPLUS ON)
swig_add_library(${PROJECT_NAME}
  TYPE MODULE
  LANGUAGE python
  SOURCES ${dot_i_srcs}
)
set_property(TARGET ${PROJECT_NAME} PROPERTY SWIG_INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Python_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  myframe
  ${Python_LIBRARIES}
)

install(TARGETS
  ${PROJECT_NAME}
  LIBRARY DESTINATION ${MYFRAME_PYTHON_DIR}
  ARCHIVE DESTINATION ${MYFRAME_PYTHON_DIR}
  RUNTIME DESTINATION ${MYFRAME_PYTHON_DIR}
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/pymyframe.py"
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
  DESTINATION ${MYFRAME_PYTHON_DIR}
)
install(PROGRAMS
  "${CMAKE_CURRENT_SOURCE_DIR}/example.py"
  DESTINATION ${MYFRAME_BIN_DIR}
)
