cmake_minimum_required(VERSION 3.25)

project(CodexOfPowerNG VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COPNG_BUILD_CPP_TEST_TARGETS "Build standalone C++ test executables for ctest" OFF)
include(CTest)

if(MSVC)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
  add_compile_options(/permissive- /Zc:__cplusplus /EHsc)
endif()

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(${PROJECT_NAME}_build_catalog STATIC
  src/BuildOptionCatalog.cpp
  include/CodexOfPowerNG/BuildOptionCatalog.h
  include/CodexOfPowerNG/BuildTypes.h
)

target_include_directories(${PROJECT_NAME}_build_catalog
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(${PROJECT_NAME}_build_state_support STATIC
  src/BuildStateStore.cpp
  src/RegistrationStateStore.cpp
  src/SerializationStateStore.cpp
  src/State.cpp
  include/CodexOfPowerNG/BuildStateStore.h
  include/CodexOfPowerNG/RegistrationStateStore.h
  include/CodexOfPowerNG/SerializationStateStore.h
  include/CodexOfPowerNG/SerializationStateStoreOps.h
  include/CodexOfPowerNG/State.h
)

target_include_directories(${PROJECT_NAME}_build_state_support
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}_build_state_support
  PUBLIC
    ${PROJECT_NAME}_build_catalog
    CommonLibSSE::CommonLibSSE
)

add_library(${PROJECT_NAME}_build_progression_support STATIC
  src/BuildProgression.cpp
  include/CodexOfPowerNG/BuildProgression.h
)

target_include_directories(${PROJECT_NAME}_build_progression_support
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}_build_progression_support
  PUBLIC
    ${PROJECT_NAME}_build_catalog
    ${PROJECT_NAME}_build_state_support
    CommonLibSSE::CommonLibSSE
)

add_library(${PROJECT_NAME}_build_effect_runtime_support STATIC
  src/BuildEffectRuntime.cpp
  include/CodexOfPowerNG/BuildEffectRuntime.h
)

target_include_directories(${PROJECT_NAME}_build_effect_runtime_support
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}_build_effect_runtime_support
  PUBLIC
    ${PROJECT_NAME}_build_catalog
    ${PROJECT_NAME}_build_state_support
    CommonLibSSE::CommonLibSSE
)

add_library(${PROJECT_NAME}_build_request_support STATIC
  src/PrismaUIRequestOpsBuild.cpp
  src/PrismaUIRequestOps.h
  src/PrismaUIInternal.h
)

target_include_directories(${PROJECT_NAME}_build_request_support
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}_build_request_support
  PUBLIC
    ${PROJECT_NAME}_build_catalog
    nlohmann_json::nlohmann_json
    CommonLibSSE::CommonLibSSE
)

add_commonlibsse_plugin(${PROJECT_NAME}
  SOURCES
    src/main.cpp
    src/Config.cpp
    src/L10n.cpp
    src/NotifiedStateStore.cpp
    src/Events.cpp
    src/Inventory.cpp
    src/PrismaUIManager.cpp
    src/PrismaUIRequests.cpp
    src/PrismaUIRequestOps.cpp
    src/PrismaUIRequestOpsBuild.cpp
    src/PrismaUISettings.cpp
    src/PrismaUISettingsPayload.cpp
    src/PrismaUIStatePayload.cpp
    src/PrismaUITogglePolicy.cpp
    src/PrismaUIToggleActions.cpp
    src/PrismaUIViewState.cpp
    src/PrismaUIViewWorkers.cpp
    src/PrismaUIViewLifecycle.cpp
    src/PrismaUIPayloadsInventory.cpp
    src/PrismaUIPayloadsBuild.cpp
    src/PrismaUIPayloadsRewards.cpp
    src/BuildEffectRuntime.cpp
    src/BuildProgression.cpp
    src/BuildStateStore.cpp
    src/RegistrationMaps.cpp
    src/RegistrationRules.cpp
    src/RegistrationStateStore.cpp
    src/RegistrationInternalMaps.cpp
    src/RegistrationInternal.cpp
    src/RegistrationInternalTcc.cpp
    src/RegistrationQuickListBuilder.cpp
    src/RegistrationQuestGuard.cpp
    src/RegistrationQuickRegister.cpp
    src/RegistrationUndo.cpp
    src/Registration.cpp
    src/RewardStateStore.cpp
    src/RewardsCore.cpp
    src/RewardsRandomTables.cpp
    src/RewardsSyncEngine.cpp
    src/Rewards.cpp
    src/SerializationSave.cpp
    src/SerializationLoad.cpp
    src/SerializationStateStore.cpp
    src/Serialization.cpp
    src/State.cpp
    src/TaskScheduler.cpp
    include/CodexOfPowerNG/Constants.h
    include/CodexOfPowerNG/BuildEffectRuntime.h
    include/CodexOfPowerNG/BuildProgression.h
    include/CodexOfPowerNG/BuildOptionCatalog.h
    include/CodexOfPowerNG/BuildStateStore.h
    include/CodexOfPowerNG/BuildTypes.h
    include/CodexOfPowerNG/Config.h
    include/CodexOfPowerNG/Events.h
    include/CodexOfPowerNG/Inventory.h
    include/CodexOfPowerNG/L10n.h
    include/CodexOfPowerNG/NotifiedStateStore.h
    include/CodexOfPowerNG/NotifiedStateStoreOps.h
    include/CodexOfPowerNG/PrismaUIManager.h
    include/CodexOfPowerNG/Registration.h
    include/CodexOfPowerNG/RegistrationFormId.h
    include/CodexOfPowerNG/RegistrationMaps.h
    include/CodexOfPowerNG/RegistrationQuestGuard.h
    include/CodexOfPowerNG/RegistrationRules.h
    include/CodexOfPowerNG/RegistrationStateStore.h
    include/CodexOfPowerNG/RegistrationUndoTypes.h
    include/CodexOfPowerNG/RewardCaps.h
    include/CodexOfPowerNG/RewardStateStore.h
    include/CodexOfPowerNG/RewardStateStoreOps.h
    include/CodexOfPowerNG/Rewards.h
    include/CodexOfPowerNG/RewardsResync.h
    include/CodexOfPowerNG/RewardsSyncRuntime.h
    include/CodexOfPowerNG/RewardsSyncPolicy.h
    include/CodexOfPowerNG/Serialization.h
    include/CodexOfPowerNG/SerializationStateStore.h
    include/CodexOfPowerNG/SerializationStateStoreOps.h
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_build_catalog)

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

if(BUILD_TESTING AND COPNG_BUILD_CPP_TEST_TARGETS)
  file(GLOB COPNG_CPP_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

  foreach(test_src IN LISTS COPNG_CPP_TEST_SOURCES)
    get_filename_component(test_name "${test_src}" NAME_WE)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" test_target_suffix "${test_name}")
    set(test_target "${PROJECT_NAME}_${test_target_suffix}")

    add_executable("${test_target}" "${test_src}")
    add_custom_target("${test_target}_test" DEPENDS "${test_target}")
    target_include_directories("${test_target}"
      PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_catalog)
    if(test_name STREQUAL "build_state_store_ops")
      target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_state_support CommonLibSSE::CommonLibSSE)
    elseif(test_name STREQUAL "build_migration_rules")
      target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_progression_support CommonLibSSE::CommonLibSSE)
    elseif(test_name STREQUAL "registration_build_progression")
      target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_progression_support CommonLibSSE::CommonLibSSE)
    elseif(test_name STREQUAL "build_effect_runtime")
      target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_effect_runtime_support CommonLibSSE::CommonLibSSE)
    elseif(test_name STREQUAL "build_request_guards")
      target_include_directories("${test_target}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
      target_link_libraries("${test_target}" PRIVATE ${PROJECT_NAME}_build_request_support CommonLibSSE::CommonLibSSE)
    endif()

    if(MSVC)
      target_compile_definitions("${test_target}" PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
      target_compile_options("${test_target}" PRIVATE /permissive- /Zc:__cplusplus /EHsc)
    endif()

    if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
      # Prefer a 64-bit Wine loader when running 64-bit Windows test executables.
      # (On some distros, `wine` may default to a 32-bit loader and fail with
      #  "could not load kernel32.dll".)
      find_program(WINE64_EXECUTABLE wine64)
      if(NOT WINE64_EXECUTABLE AND EXISTS "/usr/lib/wine/wine64")
        set(WINE64_EXECUTABLE "/usr/lib/wine/wine64")
      endif()

      find_program(WINE_EXECUTABLE wine)
      if(WINE64_EXECUTABLE)
        add_test(NAME "${test_name}" COMMAND "${WINE64_EXECUTABLE}" "$<TARGET_FILE:${test_target}>")
        set_tests_properties("${test_name}" PROPERTIES ENVIRONMENT "WINEDEBUG=fixme-all")
      elseif(WINE_EXECUTABLE)
        add_test(NAME "${test_name}" COMMAND "${WINE_EXECUTABLE}" "$<TARGET_FILE:${test_target}>")
      else()
        message(WARNING "wine not found; running ${test_name} directly may fail in cross-compile environments")
        add_test(NAME "${test_name}" COMMAND "$<TARGET_FILE:${test_target}>")
      endif()
    else()
      add_test(NAME "${test_name}" COMMAND "$<TARGET_FILE:${test_target}>")
    endif()
  endforeach()
endif()
