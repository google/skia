
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
#include <QFormLayout>

#include "SkPaint.h"

/** \class SkSettingsWidget

    The SettingsWidget contains multiple checkboxes and toggles for altering
    the visualizations.
 */
class SkSettingsWidget : public QFrame {
    Q_OBJECT

public:
    /**
        Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkSettingsWidget();


#if SK_SUPPORT_GPU
    // GL settings.
    bool isGLActive() const {
        return fGLGroup.isChecked();
    }

    int getGLSampleCount() const {
        return fGLMSAACombo.itemData(fGLMSAACombo.currentIndex()).toInt();
    }

#endif

    bool getFilterOverride(SkFilterQuality* filterQuality) const {
        int index = fFilterCombo.currentIndex();
        *filterQuality = (SkFilterQuality)fFilterCombo.itemData(index).toUInt();

        return index > 0;
    }


    // Raster settings.
    bool isRasterEnabled() {
        return fRasterGroup.isChecked();
    }

    bool isOverdrawVizEnabled() {
        return fOverdrawVizCheckBox.isChecked();
    }

    // Visualizations.
    bool isVisibilityFilterEnabled() const {
        return fVisibilityFilterCheckBox.isChecked();
    }

    bool isMegaVizEnabled() {
        return fMegaVizCheckBox.isChecked();
    }

    bool isPathOpsEnabled() {
        return fPathOpsCheckBox.isChecked();
    }

private Q_SLOTS:

Q_SIGNALS:
    void visualizationsChanged();
    void texFilterSettingsChanged();
#if SK_SUPPORT_GPU
    void glSettingsChanged();
#endif
    void rasterSettingsChanged();

private:
    QFormLayout fVerticalLayout;

    QGroupBox fVisualizationsGroup;
    QVBoxLayout fVisualizationsLayout;
    QCheckBox fVisibilityFilterCheckBox;

    QGroupBox fRasterGroup;
    QVBoxLayout fRasterLayout;
    QCheckBox fOverdrawVizCheckBox;
    QCheckBox fMegaVizCheckBox;
    QCheckBox fPathOpsCheckBox;

#if SK_SUPPORT_GPU
    QGroupBox fGLGroup;
    QFormLayout fGLLayout;
    QComboBox fGLMSAACombo;
#endif

    QComboBox fFilterCombo;
};

#endif /* SKSETTINGSWIDGET_H_ */
