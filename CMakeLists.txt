cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "Build for 10.14")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(OB-Xf VERSION 2.11.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)
option(ENABLE_ASIO "Enable ASIO support" OFF)
option(COPY_PLUGIN_AFTER_BUILD "Copy plugin after build is complete" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "Cannot enable both AddressSanitizer and ThreadSanitizer at the same time")
endif()

if(ENABLE_ASAN)
    message(STATUS "Building with AddressSanitizer enabled")

    if(MSVC)
        add_compile_options(/fsanitize=address)
        message(STATUS "Note: For MSVC, you may need to set _NO_DEBUG_HEAP=1 environment variable")
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(ENABLE_TSAN)
    if(WIN32)
        message(WARNING "ThreadSanitizer is not supported on Windows. Disabling TSAN.")
        set(ENABLE_TSAN OFF)
    else()
        message(STATUS "Building with ThreadSanitizer enabled")
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
        add_link_options(-fsanitize=thread)

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "Setting optimization level for ThreadSanitizer")
            add_compile_options(-O1)
        endif()
    endif()
endif()

if(ENABLE_ASIO)
    if(WIN32)
        if(DEFINED ENV{ASIO_SDK_HOME})
            set(ASIO_HOME $ENV{ASIO_SDK_HOME})

            if(EXISTS "${ASIO_HOME}/common/iasiodrv.h")
                message(STATUS "ASIO SDK found at: ${ASIO_HOME}")
                add_compile_definitions(JUCE_ASIO=1)
            else()
                message(WARNING "ASIO headers not found at ${ASIO_HOME}, disabling ASIO")
                set(ENABLE_ASIO OFF)
                add_compile_definitions(JUCE_ASIO=0)
            endif()
        else()
            message(WARNING "ASIO_SDK_HOME not set, disabling ASIO")
            set(ENABLE_ASIO OFF)
            add_compile_definitions(JUCE_ASIO=0)
        endif()
    else()
        message(STATUS "ASIO support only available on Windows")
        set(ENABLE_ASIO OFF)
        add_compile_definitions(JUCE_ASIO=0)
    endif()
else()
    add_compile_definitions(JUCE_ASIO=0)
endif()

add_compile_definitions(JUCE_WEB_BROWSER=0)
add_compile_definitions(JUCE_USE_CURL=0)

include(cmake/get_dependencies.cmake)

juce_add_plugin(OB-Xf
    COMPANY_NAME "surge-synth-team"
    BUNDLE_ID "org.surge-synth-team.OB-Xf"
    LV2URI "urn:org.surge-synth-team.OB-Xf"
    PLUGIN_MANUFACTURER_CODE SSTx
    PLUGIN_CODE OBXf
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "OB-Xf"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_PLUGIN_AFTER_BUILD}
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_sources(OB-Xf PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/Utils.cpp
    src/utilities/KeyCommandHandler.cpp
    src/components/ScaleComponent.cpp
    src/components/PresetBar.cpp
    src/components/SetPresetNameWindow.cpp
    src/state/StateManager.cpp
    src/midi/MidiHandler.cpp
    src/parameter/ParameterManager.cpp
)

target_include_directories(OB-Xf PRIVATE
    lib
    src
    src/components
    src/engine
    src/gui
    src/MTS
    src/interface
    src/core
    src/midi
    src/utilities
    src/parameter
    src/state
    $<$<BOOL:${ENABLE_ASIO}>:${ASIO_HOME}/common>
)

target_link_libraries(OB-Xf PRIVATE
    $<$<CONFIG:Debug>:melatonin_inspector>
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_gui_basics
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_dsp

    mts-client

    obxf_version_information
)

target_compile_definitions(OB-Xf PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0 OBXF_VERSION_STR="${PROJECT_VERSION}")

if(MSVC)
    target_compile_options(OB-Xf PRIVATE /W2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(OB-Xf PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-language-extension-token
    )
else()
    if(${CMAKE_CXX_COMPILER_VERSION} LESS 13)
        message(STATUS "gcc version < 13; dont run werror in release.")
        add_compile_options($<$<CONFIG:Release>:-w>)
    else()
        target_compile_options(OB-Xf PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

clap_juce_extensions_plugin(TARGET OB-Xf
    CLAP_ID "org.surge-synth-team.OB-Xf"
    CLAP_FEATURES instrument synthesizer "virtual analog" analog)

target_compile_definitions(OB-Xf PRIVATE)

include(cmake/basic_installer.cmake)