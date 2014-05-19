
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
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>

#include "SkPaint.h"

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

    bool getVisibilityFilter() const {
        return fVisibilityCombo.itemData(fVisibilityCombo.currentIndex()).toBool();
    }

#if SK_SUPPORT_GPU
    bool isGLActive() const {
        return fGLCheckBox.isChecked();
    }

    int getGLSampleCount() const {
        return fGLMSAACombo.itemData(fGLMSAACombo.currentIndex()).toInt();
    }

#endif

    bool getFilterOverride(SkPaint::FilterLevel* filterLevel) const {
        int index = fFilterCombo.currentIndex();
        *filterLevel = (SkPaint::FilterLevel)fFilterCombo.itemData(index).toUInt();

        return index > 0;
    }

    QCheckBox* getRasterCheckBox() {
        return &fRasterCheckBox;
    }

    QCheckBox* getOverdrawVizCheckBox() {
        return &fOverdrawVizCheckBox;
    }

    QCheckBox* getMegaVizCheckBox() {
        return &fMegaVizCheckBox;
    }

    QCheckBox* getPathOpsCheckBox() {
        return &fPathOpsCheckBox;
    }

private slots:
    void updateCommand(int newCommand);
    void updateHit(int newHit);

signals:
    void scrollingPreferences(bool isStickyActivate);
    void showStyle(bool isSingleCommand);
    void visibilityFilterChanged();
    void texFilterSettingsChanged();
#if SK_SUPPORT_GPU
    void glSettingsChanged();
#endif

private:
    QVBoxLayout mainFrameLayout;
    QFrame mainFrame;
    QVBoxLayout fVerticalLayout;

    QLabel fVisibleText;
    QFrame fVisibleFrame;
    QVBoxLayout fVisibleFrameLayout;
    QComboBox fVisibilityCombo;

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

    QHBoxLayout fVizLayout;
    QLabel fOverdrawVizLabel;
    QCheckBox fOverdrawVizCheckBox;
    QLabel fMegaVizLabel;
    QCheckBox fMegaVizCheckBox;
    QLabel fPathOpsLabel;
    QCheckBox fPathOpsCheckBox;

#if SK_SUPPORT_GPU
    QHBoxLayout fGLLayout;
    QLabel fGLLabel;
    QCheckBox fGLCheckBox;
    QGroupBox fGLMSAAButtonGroup;
    QVBoxLayout fGLMSAALayout;
    QComboBox fGLMSAACombo;
#endif

    // for filtering group
    QGroupBox fFilterButtonGroup;
    QComboBox fFilterCombo;
    QVBoxLayout fFilterLayout;

    QFrame fZoomFrame;
    QHBoxLayout fZoomLayout;
    QLabel fZoomSetting;
    QLineEdit fZoomBox;
};

#endif /* SKSETTINGSWIDGET_H_ */
