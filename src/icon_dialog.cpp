#include "icon_dialog.h"

#include "icon.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
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

    QString pathFor(int platform)
    {
        return platform == PlatformVita ? Icon::vitaPath() : Icon::pcPath();
    }
}

IconDialog::IconDialog(QWidget *parent, const QString &projectDirectory)
    : QDialog(parent)
    , m_directory(projectDirectory)
{
    setWindowTitle(QStringLiteral("Set Icon"));

    QHBoxLayout *panels = new QHBoxLayout;
    panels->addWidget(buildSlot(PlatformPc, QStringLiteral("Window icon")));
    panels->addWidget(buildSlot(PlatformVita, QStringLiteral("Vita icon")));

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
    showCurrent(PlatformPc);
    showCurrent(PlatformVita);
}

QString IconDialog::report() const
{
    return m_report;
}

IconDialog::Slot &IconDialog::slot(int platform)
{
    return platform == PlatformVita ? m_vita : m_pc;
}

QWidget *IconDialog::buildSlot(int platform, const QString &title)
{
    Slot &target = slot(platform);
    QWidget *panel = new QWidget(this);

    QLabel *heading = new QLabel(title, panel);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);

    target.preview = new QLabel(panel);
    target.preview->setFixedSize(s_previewSize, s_previewSize);
    target.preview->setAlignment(Qt::AlignCenter);
    target.preview->setFrameShape(QFrame::StyledPanel);

    QLabel *path = new QLabel(pathFor(platform), panel);
    path->setEnabled(false);

    // Wrapping to a fixed width and height keeps a longer note from resizing the panel
    // around it, which a fixed size preview cannot absorb.
    target.notes = new QLabel(panel);
    target.notes->setWordWrap(true);
    target.notes->setFixedWidth(s_previewSize * 2);
    target.notes->setFixedHeight(target.notes->fontMetrics().lineSpacing() * 3);
    target.notes->setAlignment(Qt::AlignTop);

    target.choose = new QPushButton(QStringLiteral("Choose..."), panel);
    connect(target.choose, &QPushButton::clicked, this, [this, platform] { chooseFor(platform); });

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->addWidget(heading);
    layout->addWidget(target.preview);
    layout->addWidget(path);
    layout->addWidget(target.choose);
    layout->addWidget(target.notes);
    return panel;
}

void IconDialog::chooseFor(int platform)
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
    if (platform == PlatformPc && m_sameImage->isChecked())
    {
        render(PlatformVita, source);
    }

    m_write->setEnabled(m_pc.chosen || m_vita.chosen);
}

void IconDialog::render(int platform, const QImage &source)
{
    Slot &target = slot(platform);
    target.source = source;
    target.chosen = true;

    QStringList notes;
    target.rendered = platform == PlatformVita ? Icon::forVita(source, &notes) : Icon::forPc(source, &notes);
    showSlot(platform, notes);
}

void IconDialog::showSlot(int platform, const QStringList &notes)
{
    Slot &target = slot(platform);
    QPixmap preview = QPixmap::fromImage(target.rendered);
    if (preview.width() > s_previewSize)
    {
        preview = preview.scaled(s_previewSize, s_previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    target.preview->setPixmap(preview);

    QStringList sentences;
    sentences << QStringLiteral("%1x%1.").arg(target.rendered.width());
    sentences << notes;
    target.notes->setText(sentences.join(QChar(' ')));
}

void IconDialog::showCurrent(int platform)
{
    Slot &target = slot(platform);
    target.source = QImage();
    target.rendered = QImage();
    target.chosen = false;

    const QString path = QDir(m_directory).filePath(pathFor(platform));
    const QImage existing(path);
    if (existing.isNull())
    {
        target.preview->setPixmap(QPixmap());
        target.preview->setText(QStringLiteral("none"));
        target.notes->setText(QStringLiteral("This project has no %1 yet.").arg(pathFor(platform)));
        return;
    }

    QPixmap preview = QPixmap::fromImage(existing);
    if (preview.width() > s_previewSize)
    {
        preview = preview.scaled(s_previewSize, s_previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    target.preview->setPixmap(preview);
    target.notes->setText(QStringLiteral("%1x%2 already there, and left alone.").arg(existing.width()).arg(existing.height()));
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
        render(PlatformVita, m_pc.source);
    }
    else
    {
        showCurrent(PlatformVita);
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
