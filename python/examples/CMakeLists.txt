### install
file(GLOB example_py_files "*.py")
install(PROGRAMS
  ${example_py_files}
  DESTINATION ${MYFRAME_BIN_DIR}
)
