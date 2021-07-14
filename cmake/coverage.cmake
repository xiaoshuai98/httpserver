set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include(CodeCoverage)

setup_target_for_coverage_gcovr_html(NAME code_coverage)
