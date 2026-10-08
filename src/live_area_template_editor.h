#pragma once

#include "live_area_template.h"

#include <QWidget>

#include <array>
#include <optional>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

// Edits template.xml with the rest of the form, so it is written on Save. A file
// holding more than the form shows is left alone unless replaced on purpose.
class LiveAreaTemplateEditor : public QWidget
{
    Q_OBJECT

public:
    explicit LiveAreaTemplateEditor(QWidget *parent);

    void setProjectDirectory(const QString &directory);

    bool hasPending() const;

    // Does nothing when there is nothing to write.
    bool writePending(QString *error);

signals:
    void changed();

private:
    void showTemplate(const LiveAreaTemplate &shown);
    LiveAreaTemplate templateInForm() const;
    void formEdited();
    void replaceHandEdited();
    void chooseColour(int line);
    void showColour(int line);
    void showState();

    QString m_directory;

    // What the file holds, when the form can show it.
    std::optional<LiveAreaTemplate> m_saved;
    QString m_problem;
    bool m_isReplacing = false;
    bool m_isShowing = false;
    QString m_contentRevision;

    QRadioButton *m_centre = nullptr;
    QRadioButton *m_right = nullptr;
    QWidget *m_lines = nullptr;
    std::array<QLineEdit *, LiveAreaTemplate::LineCount> m_texts = {};
    std::array<QPushButton *, LiveAreaTemplate::LineCount> m_colourButtons = {};
    std::array<QColor, LiveAreaTemplate::LineCount> m_colours = {};
    QLabel *m_status = nullptr;
    QPushButton *m_replace = nullptr;
};
