configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

# FileCheck / not come from the LLVM install (LLVM_TOOLS_BINARY_DIR); only
# targets built by this project are listed as dependencies.
set(MAGIC_TEST_DEPENDS
  magic-opt
  )

add_lit_testsuite(check-magic "Running the magic dialect regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${MAGIC_TEST_DEPENDS}
  )
set_target_properties(check-magic PROPERTIES FOLDER "Tests")

add_lit_testsuites(MAGIC ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${MAGIC_TEST_DEPENDS})
