#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// What an Android build needs that the tool downloads for itself: the NDK, one SDK platform and its build-tools, unpacked as an SDK folder, and a JDK when the machine has none.
namespace AndroidSdk
{
    QString sdkDirectory();
    QString ndkDirectory();
    QString jdkDirectory();

    // Clears what an earlier, interrupted install left, then lists the commands that download, check and unpack each missing part; finishInstall() moves them into place.
    QList<QStringList> beginInstall(bool needsJdk);
    // Fails when a part asked for, the JDK included when needsJdk, did not end up in place.
    bool finishInstall(bool unpacked, bool needsJdk, QString *error);
}
