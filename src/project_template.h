#pragma once

#include <QString>

class ProjectTemplate
{
public:
    // Fails unless the directory is empty or absent.
    static bool write(const QString &directory, const QString &name, const QString &organization, QString *error);
};
