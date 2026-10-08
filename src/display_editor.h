#pragma once

#include "project.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QSpinBox;

// Edits one display, either the one every platform shares or a platform's own.
class DisplayEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayEditor(QWidget *parent);

    DisplaySettings display() const;

    // Shows a display without reporting it as an edit.
    void setDisplay(const DisplaySettings &display);

signals:
    void edited();

private:
    void controlChanged();
    void showModeDetails();

    QComboBox *m_mode = nullptr;
    QLabel *m_modeDescription = nullptr;
    QSpinBox *m_width = nullptr;
    QSpinBox *m_height = nullptr;
    QComboBox *m_filter = nullptr;
    bool m_isShowing = false;
};
