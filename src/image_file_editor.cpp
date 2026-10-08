#include "image_file_editor.h"

#include "help_label.h"

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
    // Wide enough for the start button at its own size, which a larger image is scaled to fit.
    constexpr int s_previewWidth = 280;
}

ImageFileEditor::ImageFileEditor(QWidget *parent, const QString &title, const QString &description,
                                 const QString &path, const QSize &size, Render render)
    : QWidget(parent)
    , m_path(path)
    , m_size(size)
    , m_render(std::move(render))
{
    QLabel *heading = new QLabel(title, this);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);

    // Always the file's shape, so the page does not move as images come and go.
    const QSize previewSize = size.width() > s_previewWidth ? size.scaled(s_previewWidth, size.height(), Qt::KeepAspectRatio)
                                                            : size;
    m_preview = new QLabel(this);
    m_preview->setFixedSize(previewSize + QSize(2, 2));
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setFrameShape(QFrame::StyledPanel);

    QPushButton *choose = new QPushButton(QStringLiteral("Choose..."), this);
    connect(choose, &QPushButton::clicked, this, &ImageFileEditor::choose);

    m_revert = new QPushButton(QStringLiteral("Revert"), this);
    m_revert->setToolTip(QStringLiteral("Keep the image the project already has."));
    m_revert->setEnabled(false);
    connect(m_revert, &QPushButton::clicked, this, &ImageFileEditor::revert);

    m_remove = new QPushButton(QStringLiteral("Remove"), this);
    m_remove->setToolTip(QStringLiteral("Go without this image."));
    m_remove->hide();
    connect(m_remove, &QPushButton::clicked, this, &ImageFileEditor::remove);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(choose);
    buttons->addWidget(m_revert);
    buttons->addWidget(m_remove);
    buttons->addStretch();

    m_notes = helpLabel(QString(), this);
    m_notes->setFixedWidth(m_preview->width());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(heading);
    layout->addWidget(helpLabel(QStringLiteral("%1 %2x%3, %4.").arg(description).arg(size.width()).arg(size.height()).arg(path),
                                this));
    layout->addWidget(m_preview);
    layout->addLayout(buttons);
    layout->addWidget(m_notes);
    layout->addStretch();
}

void ImageFileEditor::setOptional(const QString &missingNote)
{
    m_missingNote = missingNote;
    m_remove->show();
}

void ImageFileEditor::setProjectDirectory(const QString &directory)
{
    m_directory = directory;
    revert();
}

bool ImageFileEditor::hasPending() const
{
    return !m_pending.isNull() || m_isRemoving;
}

ImageConversion::File ImageFileEditor::pending() const
{
    return {m_path, m_pending};
}

QString ImageFileEditor::pendingRemoval() const
{
    return m_isRemoving ? m_path : QString();
}

void ImageFileEditor::pendingWritten()
{
    if (!hasPending())
    {
        return;
    }

    m_notes->setText(m_isRemoving ? m_missingNote : QStringLiteral("Written."));
    m_remove->setEnabled(!m_isRemoving);
    m_pending = QImage();
    m_isRemoving = false;
    m_revert->setEnabled(false);
}

void ImageFileEditor::choose()
{
    const QString chosen = QFileDialog::getOpenFileName(
        this, QStringLiteral("Choose an image"), m_directory,
        QStringLiteral("Images (%1)").arg(ImageConversion::readablePatterns().join(QChar(' '))));
    if (chosen.isEmpty())
    {
        return;
    }

    QString error;
    const QImage source = ImageConversion::read(chosen, &error);
    if (source.isNull())
    {
        QMessageBox::warning(this, QStringLiteral("Choose an image"), error);
        return;
    }

    QStringList notes;
    m_pending = m_render(source, &notes);
    m_isRemoving = false;
    showImage(m_pending);

    notes.prepend(QStringLiteral("Written on Save."));
    m_notes->setText(notes.join(QChar(' ')));
    m_revert->setEnabled(true);
    m_remove->setEnabled(true);
    emit changed();
}

void ImageFileEditor::remove()
{
    const bool exists = QFileInfo::exists(QDir(m_directory).filePath(m_path));
    m_pending = QImage();
    m_isRemoving = exists;
    showImage(QImage());

    m_notes->setText(exists ? QStringLiteral("Removed on Save. %1").arg(m_missingNote) : m_missingNote);
    m_revert->setEnabled(exists);
    m_remove->setEnabled(false);
    emit changed();
}

void ImageFileEditor::revert()
{
    const bool hadPending = hasPending();
    m_pending = QImage();
    m_isRemoving = false;
    m_revert->setEnabled(false);
    m_notes->clear();

    const QImage existing(QDir(m_directory).filePath(m_path));
    m_remove->setEnabled(!existing.isNull());
    showImage(existing);
    if (existing.isNull())
    {
        m_notes->setText(m_missingNote.isEmpty() ? QStringLiteral("This project has no %1 yet.").arg(m_path)
                                                 : m_missingNote);
    }
    else if (existing.size() != m_size || existing.format() != QImage::Format_Indexed8)
    {
        fix(existing);
    }

    if (hadPending != hasPending())
    {
        emit changed();
    }
}

// The console rejects a file that is not what it expects, so one is repaired
// rather than reported, and the repair waits for Save like any other edit.
void ImageFileEditor::fix(const QImage &existing)
{
    QStringList notes;
    m_pending = m_render(existing, &notes);
    showImage(m_pending);

    const QString kind = existing.format() == QImage::Format_Indexed8 ? QStringLiteral("palette")
                                                                       : QStringLiteral("truecolor");
    notes.prepend(QStringLiteral("The file was %1x%2 %3, so it is converted on Save.")
                      .arg(existing.width())
                      .arg(existing.height())
                      .arg(kind));
    m_notes->setText(notes.join(QChar(' ')));
}

void ImageFileEditor::showImage(const QImage &image)
{
    if (image.isNull())
    {
        m_preview->setPixmap(QPixmap());
        m_preview->setText(QStringLiteral("none"));
        return;
    }

    const QSize inside = m_preview->size() - QSize(2, 2);
    m_preview->setPixmap(QPixmap::fromImage(image).scaled(inside, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
