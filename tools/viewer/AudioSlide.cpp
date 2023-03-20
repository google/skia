/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "modules/audioplayer/SkAudioPlayer.h"
#include "src/base/SkUtils.h"
#include "tools/Resources.h"
#include "tools/viewer/ClickHandlerSlide.h"

class AudioSlide : public ClickHandlerSlide {
    std::unique_ptr<SkAudioPlayer> fPlayer;
    SkRect                         fBar;

public:
    AudioSlide() { fName = "Audio"; }

    void load(SkScalar w, SkScalar h) override {
        auto data = SkData::MakeFromFileName("/Users/reed/skia/mp3/scott-joplin-peacherine-rag.mp3");
        if (data) {
            fPlayer = SkAudioPlayer::Make(data);

            SkDebugf("make: dur:%g time%g state:%d",
                     fPlayer->duration(),
                     fPlayer->time(),
                     (int)fPlayer->state());
        }

        fBar = { 10, 10, 510, 30 };
    }

    void draw(SkCanvas* canvas) override {
        if (!fPlayer) {
            return;
        }

        SkPaint p;
        p.setColor(0xFFCCCCCC);
        canvas->drawRect(fBar, p);

        p.setColor(fPlayer->isPlaying() ? SK_ColorBLUE : 0xFF8888FF);
        SkRect r = fBar;
        r.fRight = r.fLeft + (float)fPlayer->normalizedTime() * r.width();
        canvas->drawRect(r, p);
    }

    bool onChar(SkUnichar c) override {
        if (c == ' ') {
            switch (fPlayer->state()) {
                case SkAudioPlayer::State::kPlaying: fPlayer->pause(); break;
                case SkAudioPlayer::State::kPaused:  fPlayer->play(); break;
                case SkAudioPlayer::State::kStopped: fPlayer->play(); break;
            }
            return true;
        }
        return this->ClickHandlerSlide::onChar(c);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        if (fPlayer && fBar.contains(x, y)) {
            bool wasPlaying = fPlayer->isPlaying();
            if (wasPlaying) {
                fPlayer->pause();
            }
            return new Click([this, wasPlaying](Click* click) {
                if (fBar.contains(click->fCurr.fX, click->fCurr.fY)) {
                    fPlayer->setNormalizedTime((click->fCurr.fX - fBar.fLeft) / fBar.width());
                }

                if (click->fState == skui::InputState::kUp) {
                    if (wasPlaying) {
                        fPlayer->play();
                    }
                }
                return true;
            });
        }
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click*) override { return false; }

    bool animate(double /*nanos*/) override {
        return true;
    }

};

DEF_SLIDE( return new AudioSlide; )
