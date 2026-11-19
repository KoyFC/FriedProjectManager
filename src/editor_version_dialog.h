#pragma once

#include <QDialog>
#include <QProcess>
#include <QStringList>

class QLabel;
class QListWidget;
class QPushButton;

// Lists the versions of Fried Editor that were published, to choose one to download.
class EditorVersionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditorVersionDialog(QWidget *parent);
    ~EditorVersionDialog() override;

    // Empty when nothing is chosen.
    QString chosenVersion() const;

private:
    void listingFinished(int exitCode, QProcess::ExitStatus status);
    void listingFailed();
    void showVersions();
    void selectionChanged();

    QStringList m_installed;
    QProcess m_listing;

    QLabel *m_status = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_download = nullptr;
};
