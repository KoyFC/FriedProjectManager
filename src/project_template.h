#pragma once

#include <QList>
#include <QString>
#include <QStringList>

class ProjectTemplate
{
public:
    // Fails unless the directory is empty or absent.
    static bool write(const QString &directory, const QString &name, const QString &organization, QString *error);

    // Run inside a written project to make it a repository with the engine as a submodule.
    static QList<QStringList> repositoryCommands();
};
