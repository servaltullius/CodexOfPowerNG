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
    src/PrismaUIRequests.cpp
    src/PrismaUIRequestOps.cpp
    src/PrismaUISettings.cpp
    src/PrismaUISettingsPayload.cpp
    src/PrismaUIStatePayload.cpp
    src/PrismaUITogglePolicy.cpp
    src/PrismaUIToggleActions.cpp
    src/PrismaUIViewState.cpp
    src/PrismaUIViewWorkers.cpp
    src/PrismaUIViewLifecycle.cpp
    src/PrismaUIPayloadsInventory.cpp
    src/PrismaUIPayloadsRewards.cpp
    src/RegistrationMaps.cpp
    src/RegistrationRules.cpp
    src/RegistrationStateStore.cpp
    src/RegistrationInternalMaps.cpp
    src/RegistrationInternal.cpp
    src/RegistrationInternalTcc.cpp
    src/RegistrationQuickRegister.cpp
    src/Registration.cpp
    src/RewardsCore.cpp
    src/RewardsRandomTables.cpp
    src/Rewards.cpp
    src/SerializationSave.cpp
    src/SerializationLoad.cpp
    src/Serialization.cpp
    src/State.cpp
    src/TaskScheduler.cpp
    include/CodexOfPowerNG/Constants.h
    include/CodexOfPowerNG/Config.h
    include/CodexOfPowerNG/Events.h
    include/CodexOfPowerNG/Inventory.h
    include/CodexOfPowerNG/L10n.h
    include/CodexOfPowerNG/PrismaUIManager.h
    include/CodexOfPowerNG/Registration.h
    include/CodexOfPowerNG/RegistrationFormId.h
    include/CodexOfPowerNG/RegistrationMaps.h
    include/CodexOfPowerNG/RegistrationRules.h
    include/CodexOfPowerNG/RegistrationStateStore.h
    include/CodexOfPowerNG/Rewards.h
    include/CodexOfPowerNG/RewardsResync.h
    include/CodexOfPowerNG/Serialization.h
    include/CodexOfPowerNG/SerializationWriteFlow.h
    include/CodexOfPowerNG/State.h
    include/CodexOfPowerNG/TaskScheduler.h
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
