/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/CommandSet.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "src/core/SkTSort.h"

namespace sk_app {

CommandSet::CommandSet()
    : fHelpMode(kNone_HelpMode) {
    this->addCommand('h', "Overlays", "Show help screen", [this]() {
        switch (this->fHelpMode) {
            case kNone_HelpMode:
                this->fHelpMode = kGrouped_HelpMode;
                break;
            case kGrouped_HelpMode:
                this->fHelpMode = kAlphabetical_HelpMode;
                break;
            case kAlphabetical_HelpMode:
                this->fHelpMode = kNone_HelpMode;
                break;
        }
        fWindow->inval();
    });
}

void CommandSet::attach(Window* window) {
    fWindow = window;
}

bool CommandSet::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    if (Window::kDown_InputState == state) {
        for (Command& cmd : fCommands) {
            if (Command::kKey_CommandType == cmd.fType && key == cmd.fKey) {
                cmd.fFunction();
                return true;
            }
        }
    }

    return false;
}

bool CommandSet::onChar(SkUnichar c, uint32_t modifiers) {
    for (Command& cmd : fCommands) {
        if (Command::kChar_CommandType == cmd.fType && c == cmd.fChar) {
            cmd.fFunction();
            return true;
        }
    }

    return false;
}

bool CommandSet::onSoftkey(const SkString& softkey) {
    for (const Command& cmd : fCommands) {
        if (cmd.getSoftkeyString().equals(softkey)) {
            cmd.fFunction();
            return true;
        }
    }
    return false;
}

void CommandSet::addCommand(SkUnichar c, const char* group, const char* description,
                            std::function<void(void)> function) {
    fCommands.push_back(Command(c, group, description, function));
}

void CommandSet::addCommand(Window::Key k, const char* keyName, const char* group,
                            const char* description, std::function<void(void)> function) {
    fCommands.push_back(Command(k, keyName, group, description, function));
}

#if defined(SK_BUILD_FOR_WIN)
    #define SK_strcasecmp   _stricmp
#elif defined(SK_BUILD_FOR_HORIZON)
    int SK_strcasecmp(const char* s1, const char* s2)
    {
        const unsigned char *p1 = (const unsigned char *) s1;
        const unsigned char *p2 = (const unsigned char *) s2;
        int result;

        if (p1 == p2)
            return 0;

        while ((result = tolower(*p1) - tolower(*p2++)) == 0)
            if(*p1++ == '\0')
                break;
        
        return result;
    }
#else
    #define SK_strcasecmp   strcasecmp
#endif

bool CommandSet::compareCommandKey(const Command& first, const Command& second) {
    return SK_strcasecmp(first.fKeyName.c_str(), second.fKeyName.c_str()) < 0;
}

bool CommandSet::compareCommandGroup(const Command& first, const Command& second) {
    return SK_strcasecmp(first.fGroup.c_str(), second.fGroup.c_str()) < 0;
}

void CommandSet::drawHelp(SkCanvas* canvas) {
    if (kNone_HelpMode == fHelpMode) {
        return;
    }

    // Sort commands for current mode:
    SkTQSort(fCommands.begin(), fCommands.end() - 1,
             kAlphabetical_HelpMode == fHelpMode ? compareCommandKey : compareCommandGroup);

    SkFont font;
    font.setSize(16);

    SkFont groupFont;
    groupFont.setSize(18);

    SkPaint bgPaint;
    bgPaint.setColor(0xC0000000);
    canvas->drawPaint(bgPaint);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFFFFFFFF);

    SkPaint groupPaint;
    groupPaint.setAntiAlias(true);
    groupPaint.setColor(0xFFFFFFFF);

    SkScalar x = SkIntToScalar(10);
    SkScalar y = SkIntToScalar(10);

    // Measure all key strings:
    SkScalar keyWidth = 0;
    for (Command& cmd : fCommands) {
        keyWidth = SkMaxScalar(keyWidth,
                               font.measureText(cmd.fKeyName.c_str(), cmd.fKeyName.size(),
                                                SkTextEncoding::kUTF8));
    }
    keyWidth += font.measureText(" ", 1, SkTextEncoding::kUTF8);

    // If we're grouping by category, we'll be adding text height on every new group (including the
    // first), so no need to do that here. Otherwise, skip down so the first line is where we want.
    if (kGrouped_HelpMode != fHelpMode) {
        y += font.getSize();
    }

    // Print everything:
    SkString lastGroup;
    for (Command& cmd : fCommands) {
        if (kGrouped_HelpMode == fHelpMode && lastGroup != cmd.fGroup) {
            // Group change. Advance and print header:
            y += font.getSize();
            canvas->drawSimpleText(cmd.fGroup.c_str(), cmd.fGroup.size(), SkTextEncoding::kUTF8,
                                   x, y, groupFont, groupPaint);
            y += groupFont.getSize() + 2;
            lastGroup = cmd.fGroup;
        }

        canvas->drawSimpleText(cmd.fKeyName.c_str(), cmd.fKeyName.size(), SkTextEncoding::kUTF8,
                               x, y, font, paint);
        SkString text = SkStringPrintf(": %s", cmd.fDescription.c_str());
        canvas->drawString(text, x + keyWidth, y, font, paint);
        y += font.getSize() + 2;
    }
}

std::vector<SkString> CommandSet::getCommandsAsSoftkeys() const {
    std::vector<SkString> result;
    for(const Command& command : fCommands) {
        result.push_back(command.getSoftkeyString());
    }
    return result;
}

}   // namespace sk_app
