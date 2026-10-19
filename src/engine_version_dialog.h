#pragma once

#include "engine_version.h"

#include <QDialog>
#include <QList>
#include <QProcess>

class QLabel;
class QListWidget;
class QPushButton;

// Lists the engine repository's branches and tags to move the project's submodule to.
class EngineVersionDialog : public QDialog
{
    Q_OBJECT

public:
    EngineVersionDialog(QWidget *parent, const QString &projectDirectory);
    ~EngineVersionDialog() override;

    EngineVersion chosenVersion() const;

private:
    void listingFinished(int exitCode, QProcess::ExitStatus status);
    void listingFailed();
    void showVersions();
    void selectionChanged();

    QString m_currentCommit;
    QList<EngineVersion> m_versions;
    QProcess m_listing;

    QLabel *m_status = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_switch = nullptr;
};
