#pragma once

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QStackedWidget;

// What the window shows with no project open: the projects opened before.
class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent);

    void refresh();

signals:
    void projectChosen(const QString &directory);
    void newProjectRequested();
    void openProjectRequested();

private:
    void openSelected();
    void selectionChanged();

    QStackedWidget *m_area = nullptr;
    QListWidget *m_list = nullptr;
    QLabel *m_empty = nullptr;
    QPushButton *m_open = nullptr;
};
