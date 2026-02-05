set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

find_program(CMAKE_C_COMPILER clang-cl REQUIRED)
find_program(CMAKE_CXX_COMPILER clang-cl REQUIRED)
find_program(CMAKE_LINKER lld-link REQUIRED)
find_program(CMAKE_RC_COMPILER llvm-rc REQUIRED)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(_VS_MSVCTOOLS_ROOT "/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC")
set(_WINSDK_ROOT "/mnt/c/Program Files (x86)/Windows Kits/10")

if(NOT EXISTS "${_VS_MSVCTOOLS_ROOT}")
  message(FATAL_ERROR "MSVC tools not found at ${_VS_MSVCTOOLS_ROOT}")
endif()
if(NOT EXISTS "${_WINSDK_ROOT}")
  message(FATAL_ERROR "Windows SDK not found at ${_WINSDK_ROOT}")
endif()

file(GLOB _msvc_versions LIST_DIRECTORIES true "${_VS_MSVCTOOLS_ROOT}/*")
list(SORT _msvc_versions)
list(GET _msvc_versions -1 _msvc_root)

file(GLOB _winsdk_versions LIST_DIRECTORIES true "${_WINSDK_ROOT}/Include/*")
list(SORT _winsdk_versions)
list(GET _winsdk_versions -1 _winsdk_include_root)
get_filename_component(_winsdk_ver "${_winsdk_include_root}" NAME)

set(_msvc_include "${_msvc_root}/include")
set(_msvc_lib "${_msvc_root}/lib/x64")
set(_winsdk_inc_um "${_WINSDK_ROOT}/Include/${_winsdk_ver}/um")
set(_winsdk_inc_shared "${_WINSDK_ROOT}/Include/${_winsdk_ver}/shared")
set(_winsdk_inc_ucrt "${_WINSDK_ROOT}/Include/${_winsdk_ver}/ucrt")
set(_winsdk_lib_um "${_WINSDK_ROOT}/Lib/${_winsdk_ver}/um/x64")
set(_winsdk_lib_ucrt "${_WINSDK_ROOT}/Lib/${_winsdk_ver}/ucrt/x64")

set(ENV{INCLUDE} "${_msvc_include};${_winsdk_inc_um};${_winsdk_inc_shared};${_winsdk_inc_ucrt}")
set(ENV{LIB} "${_msvc_lib};${_winsdk_lib_um};${_winsdk_lib_ucrt}")

