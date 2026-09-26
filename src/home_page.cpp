#include "home_page.h"

#include "project.h"
#include "recent_projects.h"

#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QSize>
#include <QVBoxLayout>

namespace
{
    // Where each row keeps the directory it stands for.
    constexpr int s_pathRole = Qt::UserRole;

    constexpr int s_rowHeight = 44;

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

    QPushButton *create = new QPushButton(QStringLiteral("New Project..."), this);
    connect(create, &QPushButton::clicked, this, &HomePage::newProjectRequested);

    QPushButton *browse = new QPushButton(QStringLiteral("Open Project..."), this);
    connect(browse, &QPushButton::clicked, this, &HomePage::openProjectRequested);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_open);
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
        QListWidgetItem *item = new QListWidgetItem(
            QStringLiteral("%1\n%2").arg(titleOf(path), QDir::toNativeSeparators(path)), m_list);
        item->setData(s_pathRole, path);

        // Two lines of text need the room, or one row runs into the next.
        item->setSizeHint(QSize(0, s_rowHeight));
    }

    m_area->setCurrentWidget(m_list->count() == 0 ? static_cast<QWidget *>(m_empty) : m_list);
    selectionChanged();
}

void HomePage::openSelected()
{
    const QListWidgetItem *item = m_list->currentItem();
    if (item != nullptr)
    {
        emit projectChosen(item->data(s_pathRole).toString());
    }
}

void HomePage::selectionChanged()
{
    m_open->setEnabled(m_list->currentItem() != nullptr);
}
