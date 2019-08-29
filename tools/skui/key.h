// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef skui_key_DEFINED
#define skui_key_DEFINED
namespace skui {
enum class Key {
    kNONE,    //corresponds to android's UNKNOWN

    kLeftSoftKey,
    kRightSoftKey,

    kHome,    //!< the home key - added to match android
    kBack,    //!< (CLR)
    kSend,    //!< the green (talk) key
    kEnd,     //!< the red key

    k0,
    k1,
    k2,
    k3,
    k4,
    k5,
    k6,
    k7,
    k8,
    k9,
    kStar,    //!< the * key
    kHash,    //!< the # key

    kUp,
    kDown,
    kLeft,
    kRight,

    // Keys needed by ImGui
    kTab,
    kPageUp,
    kPageDown,
    kDelete,
    kEscape,
    kShift,
    kCtrl,
    kOption, // AKA Alt
    kA,
    kC,
    kV,
    kX,
    kY,
    kZ,

    kOK,      //!< the center key

    kVolUp,   //!< volume up    - match android
    kVolDown, //!< volume down  - same
    kPower,   //!< power button - same
    kCamera,  //!< camera       - same
};
}
#endif  // skui_key_DEFINED
