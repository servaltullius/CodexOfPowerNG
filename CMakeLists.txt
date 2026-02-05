cmake_minimum_required(VERSION 3.25)

project(CodexOfPowerNG VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
  add_compile_options(/permissive- /Zc:__cplusplus /EHsc)
endif()

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME}
  SOURCES
    src/main.cpp
    src/Config.cpp
    src/L10n.cpp
    src/Events.cpp
    src/Inventory.cpp
    src/PrismaUIManager.cpp
    src/Registration.cpp
    src/Rewards.cpp
    src/Serialization.cpp
    src/State.cpp
    include/CodexOfPowerNG/Constants.h
    include/CodexOfPowerNG/Config.h
    include/CodexOfPowerNG/Events.h
    include/CodexOfPowerNG/Inventory.h
    include/CodexOfPowerNG/L10n.h
    include/CodexOfPowerNG/PrismaUIManager.h
    include/CodexOfPowerNG/Registration.h
    include/CodexOfPowerNG/Rewards.h
    include/CodexOfPowerNG/Serialization.h
    include/CodexOfPowerNG/State.h
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/PrismaUI
)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "SKSE/Plugins"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SKSE/Plugins/CodexOfPowerNG"
  DESTINATION "SKSE/Plugins"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/PrismaUI/views/codexofpowerng"
  DESTINATION "PrismaUI/views"
)
