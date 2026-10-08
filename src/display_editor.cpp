#include "display_editor.h"

#include "help_label.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>

namespace
{
    constexpr int s_maxDisplaySide = 4096;
}

DisplayEditor::DisplayEditor(QWidget *parent)
    : QWidget(parent)
{
    m_mode = new QComboBox(this);
    const QList<std::array<QString, 3>> modes = {
        {QStringLiteral("Native resolution"), QStringLiteral("default"),
         QStringLiteral("Draws at the screen's own resolution, unscaled. The game adapts to whatever size it gets.")},
        {QStringLiteral("Fit"), QStringLiteral("fit"),
         QStringLiteral("Scales the game's size as large as the screen allows, keeping its shape, with black bars.")},
        {QStringLiteral("Integer scale"), QStringLiteral("integer"),
         QStringLiteral("Like Fit, but only by whole multiples, so every pixel stays the same size.")},
        {QStringLiteral("Expand"), QStringLiteral("expand"),
         QStringLiteral("Like Fit, but the game sees more on the longer side instead of black bars.")},
        {QStringLiteral("Stretch"), QStringLiteral("stretch"),
         QStringLiteral("Fills the screen with the game's size, distorting its shape.")},
    };
    for (const auto &[label, mode, description] : modes)
    {
        m_mode->addItem(label, mode);
        m_mode->setItemData(m_mode->count() - 1, description, Qt::ToolTipRole);
    }

    m_modeDescription = helpLabel(QString(), this);

    m_width = new QSpinBox(this);
    m_width->setRange(1, s_maxDisplaySide);
    m_width->setToolTip(QStringLiteral("The width the game is designed for."));
    m_height = new QSpinBox(this);
    m_height->setRange(1, s_maxDisplaySide);
    m_height->setToolTip(QStringLiteral("The height the game is designed for."));

    QHBoxLayout *size = new QHBoxLayout;
    size->addWidget(m_width);
    size->addWidget(new QLabel(QStringLiteral("x"), this));
    size->addWidget(m_height);
    size->addStretch();

    m_filter = new QComboBox(this);
    m_filter->addItem(QStringLiteral("Nearest"), QStringLiteral("nearest"));
    m_filter->setItemData(0, QStringLiteral("Keeps pixels sharp when scaled, which is what pixel art wants."),
                          Qt::ToolTipRole);
    m_filter->addItem(QStringLiteral("Linear"), QStringLiteral("linear"));
    m_filter->setItemData(1, QStringLiteral("Blends pixels when scaled, which suits art drawn at a high resolution."),
                          Qt::ToolTipRole);

    QFormLayout *form = new QFormLayout(this);
    form->setContentsMargins(0, 0, 0, 0);

    // Styles that keep fields at their size hint measure the wrapped description at
    // one width and lay it out at another, cutting off its last line.
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    QVBoxLayout *mode = new QVBoxLayout;
    mode->setSpacing(2);
    mode->addWidget(m_mode);
    mode->addWidget(m_modeDescription);

    form->addRow(QStringLiteral("Mode"), mode);
    form->addRow(QStringLiteral("Designed for"), size);
    form->addRow(QStringLiteral("Filter"), m_filter);

    connect(m_mode, &QComboBox::currentIndexChanged, this, &DisplayEditor::controlChanged);
    connect(m_width, &QSpinBox::valueChanged, this, &DisplayEditor::controlChanged);
    connect(m_height, &QSpinBox::valueChanged, this, &DisplayEditor::controlChanged);
    connect(m_filter, &QComboBox::currentIndexChanged, this, &DisplayEditor::controlChanged);

    showModeDetails();
}

DisplaySettings DisplayEditor::display() const
{
    return {
        m_mode->currentData().toString(),
        m_width->value(),
        m_height->value(),
        m_filter->currentData().toString(),
    };
}

void DisplayEditor::setDisplay(const DisplaySettings &display)
{
    m_isShowing = true;
    m_mode->setCurrentIndex(std::max(0, m_mode->findData(display.m_mode)));
    m_width->setValue(display.m_width);
    m_height->setValue(display.m_height);
    m_filter->setCurrentIndex(std::max(0, m_filter->findData(display.m_filter)));
    m_isShowing = false;

    showModeDetails();
}

void DisplayEditor::controlChanged()
{
    showModeDetails();
    if (!m_isShowing)
    {
        emit edited();
    }
}

void DisplayEditor::showModeDetails()
{
    m_modeDescription->setText(m_mode->currentData(Qt::ToolTipRole).toString());

    const bool isScaled = m_mode->currentData().toString() != QStringLiteral("default");
    m_width->setEnabled(isScaled);
    m_height->setEnabled(isScaled);
}
