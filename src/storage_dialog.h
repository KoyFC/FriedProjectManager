#pragma once

#include "downloads.h"

#include <QDialog>
#include <QList>

class QLabel;
class QPushButton;
class QTreeWidget;

// Lists what has been downloaded outside the projects, with its size, and deletes it.
class StorageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StorageDialog(QWidget *parent);

private:
    void showInstalled();
    void selectionChanged();
    void deleteSelected();
    void openSelectedFolder();
    QList<Download> selectedDownloads() const;

    QList<Download> m_downloads;

    QTreeWidget *m_list = nullptr;
    QLabel *m_total = nullptr;
    QPushButton *m_delete = nullptr;
    QPushButton *m_openFolder = nullptr;
};
