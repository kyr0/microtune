# Microtune

A MIDI FX plugin that microtunes any virtual instrument.

## Platforms and DAWs
Should be working on Windows, Mac and Linux using Reaper, Logic, Bitwig and others.
By default AAX is not built, but if you have the AAX sdk you will be able to enable this.

## Building
### Pre-requisites
- Windows, Linux, macOS based system
- C++ based developer toolchain such as Clang, VC++, etc.
- CMake
- VSCode (optional but recommended, see below)
- VST2 sdk (only if you need vst2 plugin)
- AAX (ProTools) SDK (only if you need it. disabled by default)

### Building
- Install developer tool chain on your system. Windows has been tested with MS C++, Mac has been tested with Clang, Linux with GCC on Ubuntu and Buster
- Install CMake on your system. Go to cmake.org/download
- If you want to set a particular version add Å-DAUDIOAPP_VERSION=9.9.9 in options below otherwise the version will be 0.9.0 
- VST2 - if you need it you need to add -DVST2_PATH=path-to-vst2-sdk-here to options below
- AAX - if you need it you need to install the sdk and edit the CMakefile
- AU - if you are building the AudioUnit add the option -DAUDIOAPP_IS_SYNTH=FALSE
- Build Audioapp:


`git submodule update --init --recursive`

or

`git submodule add https://github.com/juce-framework/JUCE lib/JUCE`
(in case you're setting up your own JUCE plugin from scratch)

`cmake -B build [options]`
`cmake --build build --config Release`

Also add PGM (Plugin GUI Magic):

`git submodule add --branch develop https://github.com/ffAudio/PluginGuiMagic lib/foleys_gui_magic`

### VSCode
Development is a lot easier with VSCode using the CMake extension. Simply point vscode at the root directory of the repo. It pretty much detects a cmake project and handles building without any issues.
- Install C++ extensions for vscode
- Install CMake Tools extensions for vscode

### XCode
If you want to use XCode on Mac, adjust the first cmake command to `cmake -B build -GXcode` and your build directory will contain xcode assets.

### License

GPL-3.0 License

### Contributors

Microtune has been developed by me, Aron Homberg (kyr0).

The project layout (template) and some code (build pipeline, CMake files) etc. is based on Stochas (https://github.com/surge-synthesizer/stochas).
Namely:

- Andrew Shakinovsky (rudeog)
- Paul (baconpaul)
- Maarten Kruithof (tyberion)

I'm really thankful for the open source work at Stochas!

Furthermore, I'd like to thank Daniel Walz (developer of Plugin GUI Magic) for his  
support on the "The Audio Programmer" Discord server.

The background image is by Max Bender, https://unsplash.com/photos/1YHXFeOYpN0