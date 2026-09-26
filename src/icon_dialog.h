#pragma once

#include "platform.h"

#include <QDialog>
#include <QImage>

class QCheckBox;
class QLabel;
class QPushButton;

// Shows what each platform's icon would become before any of it is written.
class IconDialog : public QDialog
{
    Q_OBJECT

public:
    IconDialog(QWidget *parent, const QString &projectDirectory);

    // What was written, once the dialog has been accepted.
    QString report() const;

private:
    struct PlatformIcon
    {
        QLabel *preview = nullptr;
        QLabel *notes = nullptr;
        QPushButton *choose = nullptr;
        QImage source;
        QImage rendered;
        bool chosen = false;
    };

    QWidget *buildPanel(Platform platform, const QString &title);
    void chooseFor(Platform platform);
    void render(Platform platform, const QImage &source);
    void showRendered(Platform platform, const QStringList &notes);
    void showOnDisk(Platform platform);
    void sameImageToggled(bool same);
    void write();

    PlatformIcon &iconFor(Platform platform);

    QString m_directory;
    QString m_report;

    PlatformIcon m_pc;
    PlatformIcon m_vita;
    QCheckBox *m_sameImage = nullptr;
    QPushButton *m_write = nullptr;
};
