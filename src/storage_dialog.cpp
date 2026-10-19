#include "storage_dialog.h"

#include "help_label.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    constexpr int s_itemIndexRole = Qt::UserRole;

    enum Column
    {
        NameColumn,
        OriginColumn,
        SizeColumn
    };

    QString sizeText(qint64 bytes)
    {
        return QLocale().formattedDataSize(bytes);
    }

    QString originText(StorageItem::Origin origin)
    {
        switch (origin)
        {
        case StorageItem::Origin::ProjectManager:
            return QStringLiteral("Downloaded by this tool");
        case StorageItem::Origin::ItsOwnInstaller:
            return QStringLiteral("Installed separately");
        default:
            return QStringLiteral("Downloaded by builds");
        }
    }
}

StorageDialog::StorageDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Storage"));
    resize(620, 420);

    m_list = new QTreeWidget(this);
    m_list->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Origin"), QStringLiteral("Size")});
    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_list->setItemsExpandable(false);
    m_list->setRootIsDecorated(false);
    m_list->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    m_list->header()->setSectionResizeMode(OriginColumn, QHeaderView::ResizeToContents);
    m_list->header()->setSectionResizeMode(SizeColumn, QHeaderView::ResizeToContents);
    m_list->header()->setStretchLastSection(false);

    m_total = new QLabel(this);
    m_total->setWordWrap(true);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_delete = buttons->addButton(QStringLiteral("Delete"), QDialogButtonBox::ActionRole);
    m_openFolder = buttons->addButton(QStringLiteral("Open Folder"), QDialogButtonBox::ActionRole);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(helpLabel(QStringLiteral("What builds use outside the projects, by platform. Deleting what was "
                                               "downloaded costs nothing but downloading it again the next time a "
                                               "build needs it. An SDK installed separately is only shown here, and "
                                               "is removed with the installer that put it there."),
                                this));
    layout->addWidget(m_list, 1);
    layout->addWidget(m_total);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_delete, &QPushButton::clicked, this, &StorageDialog::deleteSelected);
    connect(m_openFolder, &QPushButton::clicked, this, &StorageDialog::openSelectedFolder);
    connect(m_list, &QTreeWidget::itemSelectionChanged, this, &StorageDialog::selectionChanged);

    showItems();
}

// The sizes are thrown away with the dialog, but the walk still has to end before it goes.
StorageDialog::~StorageDialog()
{
    if (m_measuring != nullptr)
    {
        m_measuring->wait();
    }
}

void StorageDialog::showItems()
{
    m_list->clear();
    m_rows.clear();
    m_items = Storage::items();

    QFont groupFont = m_list->font();
    groupFont.setBold(true);

    QMap<QString, QTreeWidgetItem *> groups;
    for (int index = 0; index < m_items.size(); ++index)
    {
        const StorageItem &item = m_items[index];

        QTreeWidgetItem *&group = groups[item.platform];
        if (group == nullptr)
        {
            group = new QTreeWidgetItem(m_list, {item.platform});
            group->setFlags(Qt::ItemIsEnabled);
            group->setFont(NameColumn, groupFont);
            group->setFont(SizeColumn, groupFont);
            group->setTextAlignment(SizeColumn, Qt::AlignRight | Qt::AlignVCenter);
        }

        const QString name = item.complete ? item.name : QStringLiteral("%1 (incomplete)").arg(item.name);
        QTreeWidgetItem *row = new QTreeWidgetItem(group, {name, originText(item.origin), QStringLiteral("...")});
        row->setData(NameColumn, s_itemIndexRole, index);
        row->setTextAlignment(SizeColumn, Qt::AlignRight | Qt::AlignVCenter);
        row->setToolTip(NameColumn, QDir::toNativeSeparators(item.path));
        m_rows << row;
    }
    m_list->expandAll();

    m_total->setText(m_items.isEmpty() ? QStringLiteral("Nothing is stored outside the projects.")
                                       : QStringLiteral("Measuring..."));
    selectionChanged();
    measure();
}

// An SDK runs to gigabytes, so it is walked away from the window.
void StorageDialog::measure()
{
    if (m_measuring != nullptr)
    {
        m_measuring->wait();
        delete m_measuring;
        m_measuring = nullptr;
    }

    QStringList paths;
    for (const StorageItem &item : std::as_const(m_items))
    {
        paths << item.path;
    }

    const int generation = ++m_generation;
    m_measuring = QThread::create([this, paths, generation] {
        QList<qint64> sizes;
        for (const QString &path : paths)
        {
            sizes << Storage::sizeOf(path);
        }
        QMetaObject::invokeMethod(this, [this, generation, sizes] { showSizes(generation, sizes); },
                                  Qt::QueuedConnection);
    });
    m_measuring->start();
}

void StorageDialog::showSizes(int generation, const QList<qint64> &sizes)
{
    if (generation != m_generation)
    {
        return;
    }
    m_sizes = sizes;

    qint64 downloaded = 0;
    qint64 installedSeparately = 0;
    QMap<QTreeWidgetItem *, qint64> groupSizes;

    for (int index = 0; index < m_rows.size(); ++index)
    {
        QTreeWidgetItem *row = m_rows[index];
        row->setText(SizeColumn, sizeText(sizes[index]));
        groupSizes[row->parent()] += sizes[index];
        (m_items[index].removable() ? downloaded : installedSeparately) += sizes[index];
    }

    for (auto group = groupSizes.cbegin(); group != groupSizes.cend(); ++group)
    {
        group.key()->setText(SizeColumn, sizeText(group.value()));
    }

    m_total->setText(QStringLiteral("%1 downloaded, which can be deleted here, and %2 in SDKs installed separately.")
                         .arg(sizeText(downloaded), sizeText(installedSeparately)));
}

QList<QTreeWidgetItem *> StorageDialog::selectedRows() const
{
    QList<QTreeWidgetItem *> rows;
    for (QTreeWidgetItem *row : m_list->selectedItems())
    {
        if (row->parent() != nullptr)
        {
            rows << row;
        }
    }
    return rows;
}

const StorageItem &StorageDialog::itemOf(const QTreeWidgetItem *row) const
{
    return m_items.at(row->data(NameColumn, s_itemIndexRole).toInt());
}

void StorageDialog::selectionChanged()
{
    const QList<QTreeWidgetItem *> rows = selectedRows();

    bool allRemovable = !rows.isEmpty();
    for (const QTreeWidgetItem *row : rows)
    {
        allRemovable = allRemovable && itemOf(row).removable();
    }

    m_delete->setEnabled(allRemovable);
    m_delete->setToolTip(allRemovable || rows.isEmpty()
                             ? QString()
                             : QStringLiteral("An SDK installed separately is removed with its own installer."));
    m_openFolder->setEnabled(rows.size() == 1);
}

void StorageDialog::deleteSelected()
{
    const QList<QTreeWidgetItem *> rows = selectedRows();

    QStringList names;
    qint64 size = 0;
    bool sizeKnown = m_sizes.size() == m_rows.size();
    for (const QTreeWidgetItem *row : rows)
    {
        names << row->text(NameColumn);
        if (sizeKnown)
        {
            size += m_sizes[m_rows.indexOf(row)];
        }
    }

    const QString question = sizeKnown ? QStringLiteral("Delete %1, freeing %2?")
                                             .arg(names.join(QStringLiteral(", ")), sizeText(size))
                                       : QStringLiteral("Delete %1?").arg(names.join(QStringLiteral(", ")));
    if (QMessageBox::question(this, QStringLiteral("Delete from Storage"), question) != QMessageBox::Yes)
    {
        return;
    }

    QStringList failed;
    for (const QTreeWidgetItem *row : rows)
    {
        const StorageItem &item = itemOf(row);
        if (!QDir(item.path).removeRecursively())
        {
            failed << QDir::toNativeSeparators(item.path);
        }
    }

    showItems();

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
    const QList<QTreeWidgetItem *> rows = selectedRows();
    if (rows.size() == 1)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(itemOf(rows.first()).path));
    }
}
