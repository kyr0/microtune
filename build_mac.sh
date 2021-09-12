#
# This will build the mac install and disk image.
# you will need to have "Packages 1.2.9" installed on your system for this to work correctly
# Be sure to run this from the project's root directory
set -e
AUDIOAPP_VERSION=`cat VERSION`
cmake -Bbuild -GXcode -DAUDIOAPP_VERSION=${AUDIOAPP_VERSION} 
cmake --build build --target audioapp_VST3 --config Release
cmake --build build --target audioapp_Standalone --config Release
cmake -Bbuild -DAUDIOAPP_IS_SYNTH=FALSE -DAUDIOAPP_VERSION=${AUDIOAPP_VERSION} 
cmake --build build --target audioapp_AU --config Release
if [ "$1" == "install" ]; then
   echo building package...
   packagesbuild "install/mac/Audioapp.pkgproj"
   echo creating disk image...
   hdiutil create -volname "Audioapp ${AUDIOAPP_VERSION}" -srcfolder build.install/mac/audioapp -ov -format UDRO build.install/mac/audioapp-mac-install.dmg
fi
