
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKSETTINGSWIDGET_H_
#define SKSETTINGSWIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>

/** \class SkSettingsWidget

    The SettingsWidget contains multiple checkboxes and toggles for altering
    the visibility.
 */
class SkSettingsWidget : public QWidget {
    Q_OBJECT

public:
    /**
        Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkSettingsWidget();

    /** Sets the displayed user zoom level. A scale of 1.0 represents no zoom. */
    void setZoomText(float scale);

    QRadioButton* getVisibilityButton();

#if SK_SUPPORT_GPU
    bool isGLActive() {
        return fGLCheckBox.isChecked();
    }

    int getGLSampleCount() {
        if (fGLMSAA4On.isChecked()) {
            return 4;
        } else if (fGLMSAA16On.isChecked()) {
            return 16;
        }
        return 0;
    }

#endif

    QCheckBox* getRasterCheckBox() {
        return &fRasterCheckBox;
    }

    QCheckBox* getOverdrawVizCheckBox() {
        return &fOverdrawVizCheckBox;
    }

private slots:
    void updateCommand(int newCommand);
    void updateHit(int newHit);

signals:
    void scrollingPreferences(bool isStickyActivate);
    void showStyle(bool isSingleCommand);
    void visibilityFilter(bool isEnabled);
#if SK_SUPPORT_GPU
    void glSettingsChanged();
#endif

private:
    QVBoxLayout mainFrameLayout;
    QFrame mainFrame;
    QVBoxLayout fVerticalLayout;

    QLabel fVisibileText;
    QFrame fVisibleFrame;
    QVBoxLayout fVisibleFrameLayout;
    QRadioButton fVisibleOn;
    QRadioButton fVisibleOff;

    QLabel fCommandToggle;
    QFrame fCommandFrame;
    QVBoxLayout fCommandLayout;

    QHBoxLayout fCurrentCommandLayout;
    QLabel fCurrentCommandLabel;
    QLineEdit fCurrentCommandBox;

    QHBoxLayout fCommandHitLayout;
    QLabel fCommandHitLabel;
    QLineEdit fCommandHitBox;

    QFrame fCanvasFrame;
    QVBoxLayout fCanvasLayout;
    QLabel fCanvasToggle;

    QHBoxLayout fRasterLayout;
    QLabel fRasterLabel;
    QCheckBox fRasterCheckBox;

    QHBoxLayout fOverdrawVizLayout;
    QLabel fOverdrawVizLabel;
    QCheckBox fOverdrawVizCheckBox;

#if SK_SUPPORT_GPU
    QHBoxLayout fGLLayout;
    QLabel fGLLabel;
    QCheckBox fGLCheckBox;
    QGroupBox fGLMSAAButtonGroup;
    QVBoxLayout fGLMSAALayout;
    QRadioButton fGLMSAAOff;
    QRadioButton fGLMSAA4On;
    QRadioButton fGLMSAA16On;
#endif

    QFrame fZoomFrame;
    QHBoxLayout fZoomLayout;
    QLabel fZoomSetting;
    QLineEdit fZoomBox;
};

#endif /* SKSETTINGSWIDGET_H_ */
