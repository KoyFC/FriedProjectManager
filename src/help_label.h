#pragma once

#include <QColor>
#include <QLabel>
#include <QPalette>

// A line explaining the control above it, quieter than a label but still readable,
// which a disabled label is not.
inline QLabel *helpLabel(const QString &text, QWidget *parent)
{
    QLabel *help = new QLabel(text, parent);
    help->setWordWrap(true);

    QPalette palette = help->palette();
    QColor colour = palette.color(QPalette::WindowText);
    colour.setAlphaF(0.65f);
    palette.setColor(QPalette::WindowText, colour);
    help->setPalette(palette);
    return help;
}
