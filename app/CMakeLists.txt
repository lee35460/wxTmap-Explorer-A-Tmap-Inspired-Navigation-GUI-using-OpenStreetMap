cmake_minimum_required(VERSION 3.16)
project(wxTmapExplorer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- wxWidgets ---
find_package(wxWidgets REQUIRED COMPONENTS core base webview)
include(${wxWidgets_USE_FILE})

# --- nlohmann/json ---
find_package(nlohmann_json CONFIG REQUIRED)

# --- spdlog ---
find_package(spdlog CONFIG REQUIRED)

# --- GTest ---
enable_testing()
find_package(GTest REQUIRED)

# --- Source files ---
set(SOURCES
    app/src/AppFrame.cpp
    app/src/MapPanel.cpp
    app/src/render/RenderPipeline.cpp
    app/src/render/RenderMetricsExporter.cpp
)

set(HEADERS
    app/include/AppFrame.h
    app/include/MapPanel.h
    app/include/render/RenderPipeline.h
    app/include/render/RenderMetricsExporter.h
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE app/include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${wxWidgets_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

# --- Tests ---
add_executable(test_renderpipeline app/test/test_renderpipeline.cpp)
target_include_directories(test_renderpipeline PRIVATE app/include)
target_link_libraries(test_renderpipeline
    PRIVATE
        GTest::gtest_main
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

include(GoogleTest)
gtest_discover_tests(test_renderpipeline)