#include "home_page.h"

#include "icon.h"
#include "project.h"
#include "recent_projects.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QColor>
#include <QIcon>
#include <QImage>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QStackedWidget>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QVBoxLayout>

namespace
{
    constexpr int s_pathRole = Qt::UserRole;

    constexpr int s_missingRole = Qt::UserRole + 1;

    constexpr int s_rowHeight = 44;
    constexpr int s_iconSize = 32;

    QIcon iconOf(const QString &directory)
    {
        QPixmap canvas(s_iconSize, s_iconSize);
        canvas.fill(Qt::transparent);

        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        const QImage icon(QDir(directory).filePath(Icon::path(Platform::Pc)));
        if (!icon.isNull())
        {
            painter.drawImage(canvas.rect(), icon);
        }

        // The edge shows a white icon on a pale row, and gives a project without one the same width.
        painter.setPen(QColor(0, 0, 0, 60));
        painter.drawRect(0, 0, s_iconSize - 1, s_iconSize - 1);
        return QIcon(canvas);
    }

    QString titleOf(const QString &directory)
    {
        Project project;
        QString error;
        if (project.load(directory, &error))
        {
            return project.name();
        }

        return QFileInfo(directory).fileName();
    }
}

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *heading = new QLabel(QStringLiteral("Projects"), this);
    QFont large = heading->font();
    large.setPointSize(large.pointSize() + 4);
    large.setBold(true);
    heading->setFont(large);

    m_list = new QListWidget(this);
    m_list->setAlternatingRowColors(true);
    m_list->setIconSize(QSize(s_iconSize, s_iconSize));
    connect(m_list, &QListWidget::itemDoubleClicked, this, &HomePage::openSelected);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &HomePage::selectionChanged);

    m_empty = new QLabel(QStringLiteral("Nothing here yet. Create a project, or open one you already have."), this);
    m_empty->setWordWrap(true);
    m_empty->setEnabled(false);
    m_empty->setAlignment(Qt::AlignCenter);

    // One widget holds the space either way, so nothing shifts when the list empties.
    m_area = new QStackedWidget(this);
    m_area->addWidget(m_empty);
    m_area->addWidget(m_list);

    m_open = new QPushButton(QStringLiteral("Open"), this);
    m_open->setEnabled(false);
    connect(m_open, &QPushButton::clicked, this, &HomePage::openSelected);

    m_locate = new QPushButton(QStringLiteral("Locate..."), this);
    m_locate->setEnabled(false);
    m_locate->setToolTip(QStringLiteral("Point this entry at the project's new place."));
    connect(m_locate, &QPushButton::clicked, this, &HomePage::locateSelected);

    m_remove = new QPushButton(QStringLiteral("Remove"), this);
    m_remove->setEnabled(false);
    m_remove->setToolTip(QStringLiteral("Take the project off this list. Nothing is deleted."));
    connect(m_remove, &QPushButton::clicked, this, &HomePage::removeSelected);

    QPushButton *create = new QPushButton(QStringLiteral("New Project..."), this);
    connect(create, &QPushButton::clicked, this, &HomePage::newProjectRequested);

    QPushButton *browse = new QPushButton(QStringLiteral("Open Project..."), this);
    connect(browse, &QPushButton::clicked, this, &HomePage::openProjectRequested);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_open);
    buttons->addWidget(m_locate);
    buttons->addWidget(m_remove);
    buttons->addStretch();
    buttons->addWidget(create);
    buttons->addWidget(browse);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(heading);
    layout->addWidget(m_area, 1);
    layout->addLayout(buttons);
}

void HomePage::refresh()
{
    m_list->clear();
    for (const QString &path : RecentProjects::paths())
    {
        const bool missing = !Project::exists(path);
        const QString heading = missing ? QStringLiteral("%1 (not found)").arg(QFileInfo(path).fileName()) : titleOf(path);

        QListWidgetItem *item =
            new QListWidgetItem(QStringLiteral("%1\n%2").arg(heading, QDir::toNativeSeparators(path)), m_list);
        item->setData(s_pathRole, path);
        item->setData(s_missingRole, missing);
        item->setIcon(iconOf(path));

        if (missing)
        {
            item->setForeground(palette().brush(QPalette::Disabled, QPalette::WindowText));
        }

        // Two lines of text need the room, or one row runs into the next.
        item->setSizeHint(QSize(0, s_rowHeight));
    }

    m_area->setCurrentWidget(m_list->count() == 0 ? static_cast<QWidget *>(m_empty) : m_list);
    selectionChanged();
}

void HomePage::openSelected()
{
    if (selectedPath().isEmpty())
    {
        return;
    }

    if (selectionIsMissing())
    {
        locateSelected();
        return;
    }

    emit projectChosen(selectedPath());
}

void HomePage::locateSelected()
{
    const QString was = selectedPath();
    if (was.isEmpty())
    {
        return;
    }

    const QString now = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Locate %1").arg(QFileInfo(was).fileName()),
        QDir::cleanPath(QDir(was).filePath(QStringLiteral(".."))));
    if (now.isEmpty())
    {
        return;
    }

    if (!Project::exists(now))
    {
        QMessageBox::warning(this, QStringLiteral("Locate Project"),
                             QStringLiteral("%1 holds no project.fried.").arg(QDir::toNativeSeparators(now)));
        return;
    }

    RecentProjects::forget(was);
    RecentProjects::remember(now);
    refresh();
    select(now);
}

void HomePage::select(const QString &path)
{
    for (int row = 0; row < m_list->count(); ++row)
    {
        if (m_list->item(row)->data(s_pathRole).toString() == path)
        {
            m_list->setCurrentRow(row);
            return;
        }
    }
}

void HomePage::removeSelected()
{
    const QString path = selectedPath();
    if (path.isEmpty())
    {
        return;
    }

    const QString question = QStringLiteral("Take %1 off this list?\n\nThe project stays in %2, "
                                            "and opening it again puts it back on the list.")
                                 .arg(QFileInfo(path).fileName(), QDir::toNativeSeparators(path));
    if (QMessageBox::question(this, QStringLiteral("Remove Project"), question) != QMessageBox::Yes)
    {
        return;
    }

    RecentProjects::forget(path);
    refresh();
}

QString HomePage::selectedPath() const
{
    const QListWidgetItem *item = m_list->currentItem();
    return item == nullptr ? QString() : item->data(s_pathRole).toString();
}

bool HomePage::selectionIsMissing() const
{
    const QListWidgetItem *item = m_list->currentItem();
    return item != nullptr && item->data(s_missingRole).toBool();
}

void HomePage::selectionChanged()
{
    const bool selected = m_list->currentItem() != nullptr;
    const bool missing = selectionIsMissing();

    m_open->setEnabled(selected && !missing);
    m_locate->setEnabled(missing);
    m_remove->setEnabled(selected);
}
