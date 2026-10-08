#pragma once

#include "image_conversion.h"

#include <QImage>
#include <QWidget>

#include <functional>

class QLabel;
class QPushButton;

// One image file of a project at a fixed size. A chosen image waits to be written
// with the rest of the form, so it is saved and discarded the way a field is.
class ImageFileEditor : public QWidget
{
    Q_OBJECT

public:
    using Render = std::function<QImage(const QImage &source, QStringList *notes)>;

    ImageFileEditor(QWidget *parent, const QString &title, const QString &description, const QString &path,
                    const QSize &size, Render render);

    // Shows what the project holds and forgets any image waiting to be written.
    void setProjectDirectory(const QString &directory);

    bool hasPending() const;
    ImageConversion::File pending() const;

    // The waiting image is now what the project holds.
    void pendingWritten();

signals:
    void changed();

private:
    void choose();
    void revert();
    void fix(const QImage &existing);
    void showImage(const QImage &image);

    QString m_directory;
    QString m_path;
    QSize m_size;
    Render m_render;
    QImage m_pending;

    QLabel *m_preview = nullptr;
    QLabel *m_notes = nullptr;
    QPushButton *m_revert = nullptr;
};
