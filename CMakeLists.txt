# This script has been adapted from the example script that comes with JUCE in the cmake example dir.
# Original closed-source Audioapp code used the projucer 
#
# To build audioapp:
#   git submodule update --init --recursive
#   cmake -B build -DAUDIOAPP_VERSION=xxx, etc.
#   cmake --build build  --config Release
#
cmake_minimum_required(VERSION 3.16)

# always statically link c runtime on windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# default to 10.11 (earlier versions will fail)
if (NOT DEFINED AUDIOAPP_MAC_SDK_VER)
  set(AUDIOAPP_MAC_SDK_VER 10.11)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET ${AUDIOAPP_MAC_SDK_VER} CACHE STRING "Build for ${AUDIOAPP_MAC_SDK_VER}")

if (NOT DEFINED AUDIOAPP_VERSION)
  set (AUDIOAPP_VERSION 1.3.0)
endif()  

project(Audioapp VERSION ${AUDIOAPP_VERSION})
message( STATUS "Building Audioapp with version ${AUDIOAPP_VERSION}" )
if( APPLE )
  message( STATUS "Building using ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
endif()

# synth defaults to true
if (NOT DEFINED AUDIOAPP_IS_SYNTH)  
  set(AUDIOAPP_IS_SYNTH "TRUE")
endif()

# midi effect defaults to true
# TRUE is needed for AU and AUv3 in order for proper functioning in Logic
# but may have issues with other daw's. Specifically:
# I had some problems in cubase when this was turned on.
# Intermittent. It seems that when there are no output buffers it
# can sometimes get garbage in its buffer. Turning this off, while it
# causes the plugin to have a stereo out bus, seems to alleviate the problem.
# to rep the problem start cubase, hit F11 add audioapp as an instrument. right away
# it starts producing a buzz (although not always)

if (NOT DEFINED AUDIOAPP_IS_MIDI_EFFECT)  
  set(AUDIOAPP_IS_MIDI_EFFECT "TRUE")
endif()
message( STATUS "Is Synth: ${AUDIOAPP_IS_SYNTH}, Is Midi Effect: ${AUDIOAPP_IS_MIDI_EFFECT}")
# I'm not sure at this point whether this has any effect. I know etting it to "Instrument" causes it to show as
# VSTi in reaper whereas setting it to "Effect" causes it to show as VST
set(AUDIOAPP_CATEGORY "Effect") # original was "Instrument"
add_subdirectory(lib/JUCE)
add_subdirectory(Resources)

add_custom_target( git-info BYPRODUCTS ${CMAKE_BINARY_DIR}/geninclude/version.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -D PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
                           -D PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                           -D PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
                           -D AUDIOAPPSRC=${CMAKE_SOURCE_DIR} -D AUDIOAPPBLD=${CMAKE_BINARY_DIR}
                           -D AZURE_PIPELINE=${AZURE_PIPELINE}
                           -P ${CMAKE_SOURCE_DIR}/cmake/versiontools.cmake
                           )

# if you want vst2, you need to specify VST2_PATH in your environment
if (DEFINED VST2_PATH)
  message( STATUS "Building VST2. SDK at ${VST2_PATH}")
  juce_set_vst2_sdk_path(${VST2_PATH})
  set(VST2 "VST")
endif()

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (AudioPluginExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(audioapp
    COMPANY_NAME "YOUR COMPANY NAME"
    BUNDLE_ID "org.example.audioapp"
    DESCRIPTION "Example audio app"
    ICON_BIG   "${CMAKE_CURRENT_SOURCE_DIR}/image/app_logo_512.png"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT ${AUDIOAPP_IS_MIDI_EFFECT}  
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Appc
    FORMATS VST3 ${VST2} AU Standalone     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    VST3_CATEGORIES ${AUDIOAPP_CATEGORY}
    COPY_PLUGIN_AFTER_BUILD TRUE
    USE_LEGACY_COMPATIBILITY_PLUGIN_CODE TRUE #needed to maintain plugin code compatibility with what we've released
    VST3_CAN_REPLACE_VST2 TRUE  #hopefully this will allow old projects to load
    PRODUCT_NAME "Audioapp")        # The name of the final executable, which can differ from the target name

juce_add_module(${CMAKE_CURRENT_LIST_DIR}/lib/foleys_gui_magic)

add_dependencies( audioapp git-info )
include_directories( src )

juce_generate_juce_header(audioapp)  

target_sources(audioapp PRIVATE
    src/PluginEditor.cpp
    src/PluginProcessor.cpp

    ${CMAKE_BINARY_DIR}/geninclude/version.cpp
    )

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(audioapp
    PUBLIC
    FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1
    FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=0
    FOLEYS_ENABLE_BINARY_DATA=1
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0

    # GPL3 Plugs can disable splash screen
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0

    # Some linux options
    JUCE_JACK=1
    JUCE_ALSA=1
)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

juce_add_binary_data(assets NAMESPACE SeqImageX SOURCES 
    text/infobox.txt
    )

set_target_properties(assets PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(audioapp
  PRIVATE
  BinaryData
  assets
  foleys_gui_magic
  juce::juce_audio_basics
  juce::juce_audio_processors
  juce::juce_dsp
  juce::juce_audio_utils
  juce::juce_audio_plugin_client
  juce::juce_gui_extra
  juce::juce_cryptography
  juce::juce_opengl
  )

if( APPLE )
  add_custom_target( install-au-local )
  add_dependencies( install-au-local audioapp_AU )
  add_custom_command(
    TARGET install-au-local
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND echo "Installing audio unit locally"
    COMMAND COPY_PLUGIN_AFTER_BUILD "\${HOME}/Library/Audio/Plug-Ins/Components/Audioapp.component/"
    COMMAND auval -vt aumi Manu Appc
    )

endif()

#target_precompile_headers(audioapp PRIVATE src/Constants.h)