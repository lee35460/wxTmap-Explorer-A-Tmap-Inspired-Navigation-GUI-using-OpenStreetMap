cmake_minimum_required(VERSION 3.20)
project(wxTmapExplorer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_SYSTEM_CPR "Use system-installed cpr instead of FetchContent" OFF)
option(USE_SYSTEM_JSON "Use system-installed nlohmann_json instead of FetchContent" OFF)

# wxWidgets (requires webview component)
find_package(wxWidgets REQUIRED COMPONENTS core base net webview)
include(${wxWidgets_USE_FILE})

include(FetchContent)

# nlohmann/json
if(USE_SYSTEM_JSON)
  find_package(nlohmann_json CONFIG REQUIRED)
else()
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# cpr (HTTP client)
if(USE_SYSTEM_CPR)
  find_package(cpr CONFIG REQUIRED)
else()
  set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(CPR_FORCE_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
  )
  FetchContent_MakeAvailable(cpr)
endif()

# Sources
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
  ${SRC_DIR}/main.cpp
  ${SRC_DIR}/AppFrame.cpp
  ${SRC_DIR}/MapPanel.cpp
  ${SRC_DIR}/TmapClient.cpp
  ${SRC_DIR}/SimBridgeDummy.cpp
  ${SRC_DIR}/planners/DummyPlanner.cpp
  ${SRC_DIR}/planners/GridAStarPlanner.cpp
  ${SRC_DIR}/planners/PlannerFactory.cpp
)

add_executable(tmap_app ${SOURCES})

target_include_directories(tmap_app PRIVATE
  ${INC_DIR}
  ${INC_DIR}/planners
)

target_link_libraries(tmap_app PRIVATE
  ${wxWidgets_LIBRARIES}
)

if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(tmap_app PRIVATE nlohmann_json::nlohmann_json)
endif()

if(TARGET cpr::cpr)
  target_link_libraries(tmap_app PRIVATE cpr::cpr)
endif()

if(MSVC)
  target_compile_definitions(tmap_app PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(tmap_app PRIVATE /W4)
else()
  target_compile_options(tmap_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy UI assets and config after build
add_custom_command(TARGET tmap_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/ui
          $<TARGET_FILE_DIR:tmap_app>/ui)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../config/appsettings.json)
  add_custom_command(TARGET tmap_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/../config/appsettings.json
            $<TARGET_FILE_DIR:tmap_app>/appsettings.json)
endif()

# Install rules (optional)
install(TARGETS tmap_app RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ui DESTINATION .)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../config/appsettings.json)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../config/appsettings.json DESTINATION . RENAME appsettings.json)
endif()

message(STATUS "wxWidgets: ${wxWidgets_LIBRARIES}")
