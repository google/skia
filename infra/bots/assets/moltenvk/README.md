This asset is built from https://github.com/KhronosGroup/MoltenVK

Clone MoltenVK and open with *Xcode*:

  git clone https://github.com/KhronosGroup/MoltenVK.git
  cd MoltenVK
  ./fetchDependencies
  open MoltenVKPackaging.xcodeproj

Edit Common/MVKLogging.h and turn off MVK_LOG_LEVEL_INFO
Build the "MoltenVK (Release)" scheme
