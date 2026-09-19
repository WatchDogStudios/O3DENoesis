o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)
if(NOT PAL_TRAIT_NOESISGUI_SUPPORTED)
    return()
endif()

find_package(Noesis)
if(NOT Noesis_FOUND)
    return()
endif()

execute_process(
    COMMAND ${LY_PYTHON_CMD} ${CMAKE_CURRENT_LIST_DIR}/../Tools/generate_shaders.py
        --sdk "${NOESIS_SDK_ROOT}"
        --out "${CMAKE_CURRENT_LIST_DIR}/../Assets/Shaders/NoesisGUI/Generated"
    RESULT_VARIABLE noesis_generate_result
)
if(NOT noesis_generate_result EQUAL 0)
    message(FATAL_ERROR "NoesisGUI: shader generation failed (${noesis_generate_result}).")
endif()

ly_add_target(
    NAME ${gem_name}.API INTERFACE
    NAMESPACE Gem
    FILES_CMAKE noesisgui_api_files.cmake
    INCLUDE_DIRECTORIES INTERFACE Include
    BUILD_DEPENDENCIES INTERFACE AZ::AzCore
)

ly_add_target(
    NAME ${gem_name}.Private.Object STATIC
    NAMESPACE Gem
    FILES_CMAKE noesisgui_private_files.cmake
    TARGET_PROPERTIES O3DE_PRIVATE_TARGET TRUE
    INCLUDE_DIRECTORIES
        PRIVATE Include Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            3rdParty::Noesis
            Gem::Atom_RPI.Public
            Gem::Atom_RHI.Public
)

ly_add_target(
    NAME ${gem_name} ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE noesisgui_shared_files.cmake
    INCLUDE_DIRECTORIES PUBLIC Include PRIVATE Source
    BUILD_DEPENDENCIES
        PUBLIC Gem::${gem_name}.API
        PRIVATE Gem::${gem_name}.Private.Object
)
ly_create_alias(NAME ${gem_name}.Clients NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Servers NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Unified NAMESPACE Gem TARGETS Gem::${gem_name})
o3de_add_variant_dependencies_for_gem_dependencies(GEM_NAME ${gem_name} VARIANTS Clients Servers Unified)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME ${gem_name}.Editor.Private.Object STATIC
        NAMESPACE Gem
        FILES_CMAKE noesisgui_editor_private_files.cmake
        TARGET_PROPERTIES O3DE_PRIVATE_TARGET TRUE
        INCLUDE_DIRECTORIES PRIVATE Include Source
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                AZ::AssetBuilderSDK
                Gem::${gem_name}.Private.Object
    )
    ly_add_target(
        NAME ${gem_name}.Editor GEM_MODULE
        NAMESPACE Gem
        FILES_CMAKE noesisgui_editor_shared_files.cmake
        INCLUDE_DIRECTORIES PRIVATE Source PUBLIC Include
        BUILD_DEPENDENCIES
            PUBLIC Gem::${gem_name}.API
            PRIVATE Gem::${gem_name}.Editor.Private.Object
    )
    ly_create_alias(NAME ${gem_name}.Tools NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
    ly_create_alias(NAME ${gem_name}.Builders NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
    o3de_add_variant_dependencies_for_gem_dependencies(GEM_NAME ${gem_name} VARIANTS Tools Builders)
endif()

if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    if(PAL_TRAIT_NOESISGUI_TEST_SUPPORTED)
        ly_add_target(
            NAME ${gem_name}.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE noesisgui_tests_files.cmake
            INCLUDE_DIRECTORIES PRIVATE Tests Source Include
            BUILD_DEPENDENCIES PRIVATE AZ::AzTest AZ::AzFramework Gem::${gem_name}.Private.Object
        )
        ly_add_googletest(NAME Gem::${gem_name}.Tests)
    endif()
    if(PAL_TRAIT_BUILD_HOST_TOOLS AND PAL_TRAIT_NOESISGUI_EDITOR_TEST_SUPPORTED)
        ly_add_target(
            NAME ${gem_name}.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE noesisgui_editor_tests_files.cmake
            INCLUDE_DIRECTORIES PRIVATE Tests Source Include
            BUILD_DEPENDENCIES PRIVATE AZ::AzTest Gem::${gem_name}.Editor.Private.Object
        )
        ly_add_googletest(NAME Gem::${gem_name}.Editor.Tests)
    endif()
endif()
