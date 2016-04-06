/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef InputHandler_DEFINED
#define InputHandler_DEFINED

#include <string.h>

class InputHandler
{
public:
    InputHandler();

    void onKeyDown(unsigned char key);                  // Handle key down event
    void onKeyUp(unsigned char key);                    // Handle key up event
    void onMouseDown(unsigned int h, unsigned int v);   // Handle mouse down event
    void onMouseUp();                                   // Handle mouse up event

    bool isKeyDown(unsigned char key) const { 
        return fKeys[key]; 
    }
    bool isKeyUp(unsigned char key) const { 
        return !fKeys[key]; 
    }
    bool isKeyPressed(unsigned char key) const {
        return fKeyPressed[key];
    }

    bool isKeyReleased(unsigned char key) const {
        return fKeyReleased[key];
    }

    bool isMouseDown(unsigned int* h, unsigned int* v) const {
        if (fMouseDown)
        {
            *h = fMouseX; 
            *v = fMouseY;
            return true;
        }
        return false;
    }

    bool isMousePressed(unsigned int* h, unsigned int* v) const {
        if (fMousePressed)
        {
            *h = fMouseX; 
            *v = fMouseY;
            return true;
        }
        return false;
    }

    bool IsMouseReleased() const {
        return fMouseReleased;
    }

    inline void Update() {
        memset(fKeyPressed, 0, sizeof(bool) * 256);
        memset(fKeyReleased, 0, sizeof(bool) * 256);
        fMousePressed = false;
        fMouseReleased = false;
    }

private:
    // assumes ASCII keystrokes only
    bool fKeys[256];            
    bool fKeyPressed[256];
    bool fKeyReleased[256];

    bool fMouseDown;
    bool fMousePressed;
    bool fMouseReleased;
    unsigned int fMouseX;
    unsigned int fMouseY;

    InputHandler(const InputHandler& other);
    InputHandler& operator=(const InputHandler& other);
};

#endif
