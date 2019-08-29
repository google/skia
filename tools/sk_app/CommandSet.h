/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef CommandSet_DEFINED
#define CommandSet_DEFINED

#include "include/core/SkString.h"
#include "tools/sk_app/Window.h"

#include <functional>
#include <vector>

class SkCanvas;

namespace sk_app {

/**
 * Helper class used by applications that want to hook keypresses to trigger events.
 *
 * An app can simply store an instance of CommandSet and then use it as follows:
 * 1) Attach to the Window at initialization time.
 * 2) Register commands to be executed for characters or keys. Each command needs a Group and a
 *    description (both just strings). Commands attached to Keys (rather than characters) also need
 *    a displayable name for the Key. Finally, a function to execute when the key or character is
 *    pressed must be supplied. The easiest option to is pass in a lambda that captures [this]
 *    (your application object), and performs whatever action is desired.
 * 3) Register key and char handlers with the Window, and - depending on your state - forward those
 *    events to the CommandSet's onKey, onChar, and onSoftKey.
 * 4) At the end of your onPaint, call drawHelp, and pass in the application's canvas.

 * The CommandSet always binds 'h' to cycle through two different help screens. The first shows
 * all commands, organized by Group (with headings for each Group). The second shows all commands
 * alphabetically by key/character.
 */
class CommandSet {
public:
    CommandSet();

    void attach(Window* window);
    bool onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers);
    bool onChar(SkUnichar, skui::ModifierKey modifiers);
    bool onSoftkey(const SkString& softkey);

    void addCommand(SkUnichar c, const char* group, const char* description,
                    std::function<void(void)> function);
    void addCommand(skui::Key k, const char* keyName, const char* group, const char* description,
                    std::function<void(void)> function);

    void drawHelp(SkCanvas* canvas);

    std::vector<SkString> getCommandsAsSoftkeys() const;

private:
    struct Command {
        enum CommandType {
            kChar_CommandType,
            kKey_CommandType,
        };

        Command(SkUnichar c, const char* group, const char* description,
                std::function<void(void)> function)
            : fType(kChar_CommandType)
            , fChar(c)
            , fKeyName(' ' == c ? SkString("Space") : SkStringPrintf("%c", c))
            , fGroup(group)
            , fDescription(description)
            , fFunction(function) {}

        Command(skui::Key k, const char* keyName, const char* group, const char* description,
                std::function<void(void)> function)
            : fType(kKey_CommandType)
            , fKey(k)
            , fKeyName(keyName)
            , fGroup(group)
            , fDescription(description)
            , fFunction(function) {}

        CommandType fType;

        // For kChar_CommandType
        SkUnichar fChar;

        // For kKey_CommandType
        skui::Key fKey;

        // Common to all command types
        SkString fKeyName;
        SkString fGroup;
        SkString fDescription;
        std::function<void(void)> fFunction;

        SkString getSoftkeyString() const {
            return SkStringPrintf("%s (%s)", fKeyName.c_str(), fDescription.c_str());
        }
    };

    static bool compareCommandKey(const Command& first, const Command& second);
    static bool compareCommandGroup(const Command& first, const Command& second);

    enum HelpMode {
        kNone_HelpMode,
        kGrouped_HelpMode,
        kAlphabetical_HelpMode,
    };

    Window*           fWindow;
    SkTArray<Command> fCommands;
    HelpMode          fHelpMode;
};

}   // namespace sk_app

#endif
