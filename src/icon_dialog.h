#pragma once

#include "build.h"

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
    struct Slot
    {
        QLabel *preview = nullptr;
        QLabel *notes = nullptr;
        QPushButton *choose = nullptr;
        QImage source;
        QImage rendered;
        bool chosen = false;
    };

    QWidget *buildSlot(int platform, const QString &title);
    void chooseFor(int platform);
    void render(int platform, const QImage &source);
    void showSlot(int platform, const QStringList &notes);
    void showCurrent(int platform);
    void sameImageToggled(bool same);
    void write();

    Slot &slot(int platform);

    QString m_directory;
    QString m_report;

    Slot m_pc;
    Slot m_vita;
    QCheckBox *m_sameImage = nullptr;
    QPushButton *m_write = nullptr;
};
