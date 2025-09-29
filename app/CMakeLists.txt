cmake_minimum_required(VERSION 3.16)
project(wxTmapExplorer LANGUAGES CXX)

# --- macOS architecture & search path hygiene ---------------------------------
if(APPLE)
  # Build everything for Apple Silicon to match Homebrew-installed libs
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

  # Prefer Homebrew prefixes and avoid picking up Conda-provided libs (fmt/gtest)
  list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew")
  list(APPEND CMAKE_IGNORE_PREFIX_PATH "$ENV{HOME}/miniconda3" "$ENV{HOME}/anaconda3")

  # Be strict about finding frameworks/libs only for the selected arch
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- wxWidgets ---
find_package(wxWidgets REQUIRED COMPONENTS core base webview)
include(${wxWidgets_USE_FILE})

# --- Dependencies ---
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(FetchContent)

# fmt (needed by spdlog on some systems)
find_package(fmt CONFIG QUIET)
if (NOT fmt_FOUND)
  message(STATUS "fmt not found; fetching via FetchContent")
  FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
  )
  FetchContent_MakeAvailable(fmt)
endif()

# --- Testing ---
include(CTest)
if (BUILD_TESTING)
  find_package(GTest CONFIG QUIET)
  if (NOT GTest_FOUND)
    message(STATUS "GTest not found; using system or previously installed one if available")
    find_package(GTest REQUIRED)  # fallback to non-config find
  endif()
endif()

# --- Sources (app executable; do NOT list render/*.cpp here) ---
set(SOURCES
    src/main.cpp
    src/AppFrame.cpp
    src/MapPanel.cpp
    src/TmapClient.cpp
    src/SimBridgeDummy.cpp
    src/planners/DummyPlanner.cpp
    src/planners/GridAStarPlanner.cpp
    src/planners/PlannerFactory.cpp
)

set(HEADERS
    include/AppFrame.h
    include/MapPanel.h
    include/TmapClient.h
    include/IRoutePlanner.h
    include/IsimBridge.h
    include/Types.h
    include/planners/DummyPlanner.h
    include/planners/GridAStarPlanner.h
    include/planners/PlannerFactory.h
    include/render/RenderPipeline.h
    include/render/RenderMetricsExporter.h
)

# --- Render core library (shared by exe & tests) ---
add_library(render_core STATIC
    src/render/RenderPipeline.cpp
    src/render/RenderMetricsExporter.cpp
)
target_include_directories(render_core PUBLIC include)
target_link_libraries(render_core
    PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog_header_only
        $<IF:$<TARGET_EXISTS:fmt::fmt>,fmt::fmt,fmt::fmt-header-only>
)
target_compile_definitions(render_core PUBLIC FMT_HEADER_ONLY=1)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog_header_only
        $<IF:$<TARGET_EXISTS:fmt::fmt>,fmt::fmt,fmt::fmt-header-only>
        render_core
)
target_compile_definitions(${PROJECT_NAME} PRIVATE FMT_HEADER_ONLY=1)

# --- Tests ---
include(CTest)
if (BUILD_TESTING)
  find_package(GTest CONFIG QUIET)
  if (NOT GTest_FOUND)
    message(STATUS "GTest not found; using system or previously installed one if available")
    find_package(GTest REQUIRED)
  endif()

  # Collect all test sources under test/*.cpp
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp)

  add_executable(unit_tests ${TEST_SOURCES})
  target_include_directories(unit_tests PRIVATE include)
  target_link_libraries(unit_tests
      PRIVATE
          GTest::gtest_main
          render_core
          nlohmann_json::nlohmann_json
          spdlog::spdlog_header_only
          $<IF:$<TARGET_EXISTS:fmt::fmt>,fmt::fmt,fmt::fmt-header-only>
  )
  target_compile_definitions(unit_tests PRIVATE FMT_HEADER_ONLY=1)

  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()