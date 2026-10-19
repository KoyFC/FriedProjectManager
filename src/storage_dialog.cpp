#include "storage_dialog.h"

#include "help_label.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    constexpr int s_downloadIndexRole = Qt::UserRole;
    constexpr int s_sizeRole = Qt::UserRole + 1;

    enum Column
    {
        NameColumn,
        KindColumn,
        SizeColumn
    };

    QString sizeText(qint64 bytes)
    {
        return QLocale().formattedDataSize(bytes);
    }
}

StorageDialog::StorageDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Storage"));
    resize(560, 360);

    m_list = new QTreeWidget(this);
    m_list->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Kind"), QStringLiteral("Size")});
    m_list->setRootIsDecorated(false);
    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_list->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    m_list->header()->setSectionResizeMode(KindColumn, QHeaderView::ResizeToContents);
    m_list->header()->setSectionResizeMode(SizeColumn, QHeaderView::ResizeToContents);
    m_list->header()->setStretchLastSection(false);

    m_total = new QLabel(this);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_delete = buttons->addButton(QStringLiteral("Delete"), QDialogButtonBox::ActionRole);
    m_openFolder = buttons->addButton(QStringLiteral("Open Folder"), QDialogButtonBox::ActionRole);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(helpLabel(QStringLiteral("What the engine's builds and the Project Manager downloaded outside "
                                               "any project. Deleting one costs nothing but downloading it again "
                                               "the next time a build needs it."),
                                this));
    layout->addWidget(m_list, 1);
    layout->addWidget(m_total);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_delete, &QPushButton::clicked, this, &StorageDialog::deleteSelected);
    connect(m_openFolder, &QPushButton::clicked, this, &StorageDialog::openSelectedFolder);
    connect(m_list, &QTreeWidget::itemSelectionChanged, this, &StorageDialog::selectionChanged);

    showInstalled();
}

void StorageDialog::showInstalled()
{
    m_list->clear();
    m_downloads = Downloads::installed();

    qint64 total = 0;
    for (int index = 0; index < m_downloads.size(); ++index)
    {
        const Download &download = m_downloads[index];
        const qint64 size = Downloads::sizeOf(download.path);
        total += size;

        const QString name =
            download.complete ? download.name : QStringLiteral("%1 (incomplete)").arg(download.name);

        QTreeWidgetItem *item = new QTreeWidgetItem(m_list, {name, download.kind, sizeText(size)});
        item->setData(NameColumn, s_downloadIndexRole, index);
        item->setData(NameColumn, s_sizeRole, size);
        item->setTextAlignment(SizeColumn, Qt::AlignRight | Qt::AlignVCenter);
        item->setToolTip(NameColumn, QDir::toNativeSeparators(download.path));
    }

    m_total->setText(m_downloads.isEmpty() ? QStringLiteral("Nothing has been downloaded.")
                                           : QStringLiteral("%1 in total.").arg(sizeText(total)));
    selectionChanged();
}

void StorageDialog::selectionChanged()
{
    const int selected = m_list->selectedItems().size();
    m_delete->setEnabled(selected > 0);
    m_openFolder->setEnabled(selected == 1);
}

QList<Download> StorageDialog::selectedDownloads() const
{
    QList<Download> selected;
    for (const QTreeWidgetItem *item : m_list->selectedItems())
    {
        selected << m_downloads.at(item->data(NameColumn, s_downloadIndexRole).toInt());
    }
    return selected;
}

void StorageDialog::deleteSelected()
{
    const QList<Download> selected = selectedDownloads();

    QStringList names;
    qint64 size = 0;
    for (const QTreeWidgetItem *item : m_list->selectedItems())
    {
        names << item->text(NameColumn);
        size += item->data(NameColumn, s_sizeRole).toLongLong();
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("Delete from Storage"),
        QStringLiteral("Delete %1, freeing %2?").arg(names.join(QStringLiteral(", ")), sizeText(size)));
    if (answer != QMessageBox::Yes)
    {
        return;
    }

    QStringList failed;
    for (const Download &download : selected)
    {
        if (!QDir(download.path).removeRecursively())
        {
            failed << QDir::toNativeSeparators(download.path);
        }
    }

    showInstalled();

    if (!failed.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Delete from Storage"),
                             QStringLiteral("Some files could not be deleted, perhaps because a program is using "
                                            "them:\n\n%1")
                                 .arg(failed.join(QChar('\n'))));
    }
}

void StorageDialog::openSelectedFolder()
{
    const QList<Download> selected = selectedDownloads();
    if (selected.size() == 1)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(selected.first().path));
    }
}
