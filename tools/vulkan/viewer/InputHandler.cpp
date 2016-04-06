/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "InputHandler.h"
#include <ctype.h>

InputHandler::InputHandler() : fMouseDown(false), fMousePressed(false), fMouseReleased(false)
                             , fMouseX(0), fMouseY(0) {
    // clear key states
    memset(fKeys, 0, sizeof(bool) * 256);
    memset(fKeyPressed, 0, sizeof(bool) * 256);
    memset(fKeyReleased, 0, sizeof(bool) * 256);
}

void InputHandler::onKeyDown(unsigned char key) {
    if (!fKeys[key]) {
        fKeys[key] = true;
        fKeyPressed[key] = true;
    }
} 

void InputHandler::onKeyUp(unsigned char key) {
    if (fKeys[key]) {
        fKeys[key] = false;
        fKeyReleased[key] = true;
    }
}

void InputHandler::onMouseDown(unsigned int h, unsigned int v) {
    if (!fMouseDown) {
        fMouseDown = true;
        fMousePressed = true;
    }

    fMouseX = h;
    fMouseY = v;
}

void InputHandler::onMouseUp() {
    if (fMouseDown)  {
        fMouseDown = false;
        fMouseReleased = true;
    }
}
