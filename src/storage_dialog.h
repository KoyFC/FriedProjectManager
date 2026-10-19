#pragma once

#include "storage.h"

#include <QDialog>
#include <QList>

class QLabel;
class QPushButton;
class QThread;
class QTreeWidget;
class QTreeWidgetItem;

// Lists what builds depend on outside the projects, by platform and with its
// size, and deletes whatever of it was downloaded.
class StorageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StorageDialog(QWidget *parent);
    ~StorageDialog() override;

private:
    void showItems();
    void measure();
    void showSizes(int generation, const QList<qint64> &sizes);
    void selectionChanged();
    void deleteSelected();
    void openSelectedFolder();
    QList<QTreeWidgetItem *> selectedRows() const;
    const StorageItem &itemOf(const QTreeWidgetItem *row) const;

    QList<StorageItem> m_items;
    QList<QTreeWidgetItem *> m_rows;
    QList<qint64> m_sizes;
    QThread *m_measuring = nullptr;
    int m_generation = 0;

    QTreeWidget *m_list = nullptr;
    QLabel *m_total = nullptr;
    QPushButton *m_delete = nullptr;
    QPushButton *m_openFolder = nullptr;
};
