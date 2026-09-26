#include "icon_dialog.h"

#include "icon.h"

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
    // The Vita icon is shown at its own size, so its palette is visible rather than smoothed over.
    constexpr int s_previewSize = 128;

    QPixmap previewOf(const QImage &image)
    {
        const QPixmap preview = QPixmap::fromImage(image);
        return preview.width() > s_previewSize
                   ? preview.scaled(s_previewSize, s_previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                   : preview;
    }

    QString pathFor(Platform platform)
    {
        return platform == Platform::Vita ? Icon::vitaPath() : Icon::pcPath();
    }
}

IconDialog::IconDialog(QWidget *parent, const QString &projectDirectory)
    : QDialog(parent)
    , m_directory(projectDirectory)
{
    setWindowTitle(QStringLiteral("Set Icon"));

    QHBoxLayout *panels = new QHBoxLayout;
    panels->addWidget(buildPanel(Platform::Pc, QStringLiteral("Window icon")));
    panels->addWidget(buildPanel(Platform::Vita, QStringLiteral("Vita icon")));

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

    m_vita.choose->setEnabled(false);
    showOnDisk(Platform::Pc);
    showOnDisk(Platform::Vita);
}

QString IconDialog::report() const
{
    return m_report;
}

IconDialog::PlatformIcon &IconDialog::iconFor(Platform platform)
{
    return platform == Platform::Vita ? m_vita : m_pc;
}

QWidget *IconDialog::buildPanel(Platform platform, const QString &title)
{
    PlatformIcon &icon = iconFor(platform);
    QWidget *panel = new QWidget(this);

    QLabel *heading = new QLabel(title, panel);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);

    icon.preview = new QLabel(panel);
    icon.preview->setFixedSize(s_previewSize, s_previewSize);
    icon.preview->setAlignment(Qt::AlignCenter);
    icon.preview->setFrameShape(QFrame::StyledPanel);

    QLabel *path = new QLabel(pathFor(platform), panel);
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
                                     QStringLiteral("Images (%1)").arg(Icon::readablePatterns().join(QChar(' '))));
    if (chosen.isEmpty())
    {
        return;
    }

    QString error;
    const QImage source = Icon::read(chosen, &error);
    if (source.isNull())
    {
        QMessageBox::warning(this, windowTitle(), error);
        return;
    }

    render(platform, source);
    if (platform == Platform::Pc && m_sameImage->isChecked())
    {
        render(Platform::Vita, source);
    }

    m_write->setEnabled(m_pc.chosen || m_vita.chosen);
}

void IconDialog::render(Platform platform, const QImage &source)
{
    PlatformIcon &icon = iconFor(platform);
    icon.source = source;
    icon.chosen = true;

    QStringList notes;
    icon.rendered = platform == Platform::Vita ? Icon::forVita(source, &notes) : Icon::forPc(source, &notes);
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

    const QString path = QDir(m_directory).filePath(pathFor(platform));
    const QImage existing(path);
    if (existing.isNull())
    {
        icon.preview->setPixmap(QPixmap());
        icon.preview->setText(QStringLiteral("none"));
        icon.notes->setText(QStringLiteral("This project has no %1 yet.").arg(pathFor(platform)));
        return;
    }

    icon.preview->setPixmap(previewOf(existing));
    icon.notes->setText(QStringLiteral("%1x%2 already there, and left alone.").arg(existing.width()).arg(existing.height()));
}

void IconDialog::sameImageToggled(bool same)
{
    m_vita.choose->setEnabled(!same);
    if (!same)
    {
        return;
    }

    if (m_pc.chosen)
    {
        render(Platform::Vita, m_pc.source);
    }
    else
    {
        showOnDisk(Platform::Vita);
    }
    m_write->setEnabled(m_pc.chosen || m_vita.chosen);
}

void IconDialog::write()
{
    QString error;
    if (!Icon::write(m_directory, m_pc.rendered, m_vita.rendered, &error))
    {
        QMessageBox::warning(this, windowTitle(), error);
        return;
    }

    m_report = Icon::describe(m_pc.rendered, m_vita.rendered);
    accept();
}
