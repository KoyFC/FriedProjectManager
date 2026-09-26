#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    NewProjectDialog(QWidget *parent, const QString &location);

    QString directory() const;
    QString name() const;
    QString organization() const;

private:
    void chooseLocation();
    void refresh();
    QString problem() const;

    QLineEdit *m_location = nullptr;
    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLabel *m_message = nullptr;
    QPushButton *m_create = nullptr;
};
