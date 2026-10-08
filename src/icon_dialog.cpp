#include "icon_dialog.h"

#include "icon.h"
#include "image_conversion.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    // A console icon is shown at its own size, so its palette is visible rather than smoothed over.
    constexpr int s_previewSize = 128;

    // Left to right, with the window icon first because it is the one the others
    // can be derived from.
    const QList<Platform> s_panels = {Platform::Pc, Platform::Vita, Platform::Switch, Platform::Nintendo3ds, Platform::Cg50};

    QPixmap previewOf(const QImage &image)
    {
        const QPixmap preview = QPixmap::fromImage(image);
        return preview.width() > s_previewSize
                   ? preview.scaled(s_previewSize, s_previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                   : preview;
    }

    QString titleFor(Platform platform)
    {
        if (platform == Platform::Vita)
        {
            return QStringLiteral("Vita icon");
        }

        if (platform == Platform::Switch)
        {
            return QStringLiteral("Switch icon");
        }

        if (platform == Platform::Nintendo3ds)
        {
            return QStringLiteral("3DS icon");
        }

        if (platform == Platform::Cg50)
        {
            return QStringLiteral("fx-CG50 icon");
        }

        return QStringLiteral("Window icon");
    }
}

IconDialog::IconDialog(QWidget *parent, const QString &projectDirectory)
    : QDialog(parent)
    , m_directory(projectDirectory)
{
    setWindowTitle(QStringLiteral("Set Icon"));

    QHBoxLayout *panels = new QHBoxLayout;
    for (const Platform platform : s_panels)
    {
        panels->addWidget(buildPanel(platform));
    }

    m_sameImage = new QCheckBox(QStringLiteral("Same image as the window icon"), this);
    m_sameImage->setChecked(true);
    connect(m_sameImage, &QCheckBox::toggled, this, &IconDialog::sameImageToggled);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_write = buttons->addButton(QStringLiteral("Write"), QDialogButtonBox::AcceptRole);
    m_write->setEnabled(false);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_write, &QPushButton::clicked, this, &IconDialog::write);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(panels);
    layout->addWidget(m_sameImage);
    layout->addWidget(buttons);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    for (const Platform platform : s_panels)
    {
        if (platform != Platform::Pc)
        {
            iconFor(platform).choose->setEnabled(false);
        }
        showOnDisk(platform);
    }
}

QString IconDialog::report() const
{
    return m_report;
}

IconDialog::PlatformIcon &IconDialog::iconFor(Platform platform)
{
    return m_icons[platform];
}

bool IconDialog::anyChosen() const
{
    for (const PlatformIcon &icon : m_icons)
    {
        if (icon.chosen)
        {
            return true;
        }
    }

    return false;
}

QWidget *IconDialog::buildPanel(Platform platform)
{
    PlatformIcon &icon = iconFor(platform);
    QWidget *panel = new QWidget(this);

    QLabel *heading = new QLabel(titleFor(platform), panel);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);

    icon.preview = new QLabel(panel);
    icon.preview->setFixedSize(s_previewSize, s_previewSize);
    icon.preview->setAlignment(Qt::AlignCenter);
    icon.preview->setFrameShape(QFrame::StyledPanel);

    QLabel *path = new QLabel(Icon::path(platform), panel);
    path->setEnabled(false);

    // Fixed, so a longer note cannot resize the panel around a preview that will not shrink.
    icon.notes = new QLabel(panel);
    icon.notes->setWordWrap(true);
    icon.notes->setFixedWidth(s_previewSize * 2);
    icon.notes->setFixedHeight(icon.notes->fontMetrics().lineSpacing() * 3);
    icon.notes->setAlignment(Qt::AlignTop);

    icon.choose = new QPushButton(QStringLiteral("Choose..."), panel);
    connect(icon.choose, &QPushButton::clicked, this, [this, platform] { chooseFor(platform); });

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->addWidget(heading);
    layout->addWidget(icon.preview);
    layout->addWidget(path);
    layout->addWidget(icon.choose);
    layout->addWidget(icon.notes);
    return panel;
}

void IconDialog::chooseFor(Platform platform)
{
    const QString chosen =
        QFileDialog::getOpenFileName(this, QStringLiteral("Choose an icon image"), m_directory,
                                     QStringLiteral("Images (%1)").arg(ImageConversion::readablePatterns().join(QChar(' '))));
    if (chosen.isEmpty())
    {
        return;
    }

    QString error;
    const QImage source = ImageConversion::read(chosen, &error);
    if (source.isNull())
    {
        QMessageBox::warning(this, windowTitle(), error);
        return;
    }

    render(platform, source);
    if (platform == Platform::Pc && m_sameImage->isChecked())
    {
        for (const Platform other : s_panels)
        {
            if (other != Platform::Pc)
            {
                render(other, source);
            }
        }
    }

    m_write->setEnabled(anyChosen());
}

void IconDialog::render(Platform platform, const QImage &source)
{
    PlatformIcon &icon = iconFor(platform);
    icon.source = source;
    icon.chosen = true;

    QStringList notes;
    icon.rendered = Icon::render(platform, source, &notes);
    showRendered(platform, notes);
}

void IconDialog::showRendered(Platform platform, const QStringList &notes)
{
    PlatformIcon &icon = iconFor(platform);
    icon.preview->setPixmap(previewOf(icon.rendered));

    QStringList sentences;
    sentences << QStringLiteral("%1x%1.").arg(icon.rendered.width());
    sentences << notes;
    icon.notes->setText(sentences.join(QChar(' ')));
}

void IconDialog::showOnDisk(Platform platform)
{
    PlatformIcon &icon = iconFor(platform);
    icon.source = QImage();
    icon.rendered = QImage();
    icon.chosen = false;

    const QString path = QDir(m_directory).filePath(Icon::path(platform));
    const QImage existing(path);
    if (existing.isNull())
    {
        icon.preview->setPixmap(QPixmap());
        icon.preview->setText(QStringLiteral("none"));
        icon.notes->setText(QStringLiteral("This project has no %1 yet.").arg(Icon::path(platform)));
        return;
    }

    icon.preview->setPixmap(previewOf(existing));
    icon.notes->setText(QStringLiteral("%1x%2 already there, and left alone.").arg(existing.width()).arg(existing.height()));
}

void IconDialog::sameImageToggled(bool same)
{
    for (const Platform platform : s_panels)
    {
        if (platform != Platform::Pc)
        {
            iconFor(platform).choose->setEnabled(!same);
        }
    }

    if (!same)
    {
        return;
    }

    const PlatformIcon &pc = iconFor(Platform::Pc);
    const QImage source = pc.source;
    const bool chosen = pc.chosen;

    for (const Platform platform : s_panels)
    {
        if (platform == Platform::Pc)
        {
            continue;
        }

        if (chosen)
        {
            render(platform, source);
        }
        else
        {
            showOnDisk(platform);
        }
    }

    m_write->setEnabled(anyChosen());
}

void IconDialog::write()
{
    QMap<Platform, QImage> rendered;
    for (auto icon = m_icons.constBegin(); icon != m_icons.constEnd(); ++icon)
    {
        rendered.insert(icon.key(), icon.value().rendered);
    }

    QString error;
    if (!Icon::write(m_directory, rendered, &error))
    {
        QMessageBox::warning(this, windowTitle(), error);
        return;
    }

    m_report = Icon::describe(rendered);
    accept();
}
