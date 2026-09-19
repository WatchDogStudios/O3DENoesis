set(NOESIS_SDK_ROOT "$ENV{NOESIS_SDK_ROOT}" CACHE PATH "Root of the NoesisGUI SDK (contains Include/, Lib/, Bin/).")

if(TARGET 3rdParty::Noesis)
    set(Noesis_FOUND TRUE)
    return()
endif()

set(_noesis_platform_dir windows_x86_64)
set(_noesis_dll "${NOESIS_SDK_ROOT}/Bin/${_noesis_platform_dir}/Noesis.dll")
set(_noesis_lib "${NOESIS_SDK_ROOT}/Lib/${_noesis_platform_dir}/Noesis.lib")

if(NOT EXISTS "${NOESIS_SDK_ROOT}/Include/NsCore/Noesis.h" OR NOT EXISTS "${_noesis_lib}" OR NOT EXISTS "${_noesis_dll}")
    message(WARNING "NoesisGUI: SDK not found at NOESIS_SDK_ROOT='${NOESIS_SDK_ROOT}'. The NoesisGUI gem will not be built.")
    set(Noesis_FOUND FALSE)
    return()
endif()

add_library(3rdParty::Noesis SHARED IMPORTED GLOBAL)
set_target_properties(3rdParty::Noesis PROPERTIES
    IMPORTED_LOCATION "${_noesis_dll}"
    IMPORTED_IMPLIB "${_noesis_lib}"
    INTERFACE_INCLUDE_DIRECTORIES "${NOESIS_SDK_ROOT}/Include"
)
set(Noesis_FOUND TRUE)
