#include "build_bar.h"

#include <QAction>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    // Stands for the project's own directory, so the field is one editable path
    // whichever side of the project it points at.
    const QString s_projectToken = QStringLiteral("{project}");

    int comboIndexOf(Platform platform)
    {
        return static_cast<int>(platform);
    }

    int comboIndexOf(Build::Type type)
    {
        return static_cast<int>(type);
    }

    // A project path holds separators, so it is percent encoded to stay one key.
    QString rememberedKey(const QString &setting, const QString &project, Platform platform)
    {
        return QStringLiteral("%1/%2/%3")
            .arg(setting, platformKey(platform), QString::fromUtf8(QUrl::toPercentEncoding(project)));
    }

    QString directoryKey(const QString &project, Platform platform)
    {
        return rememberedKey(QStringLiteral("buildDirectories"), project, platform);
    }

    QString typeKey(const QString &project, Platform platform)
    {
        return rememberedKey(QStringLiteral("buildTypes"), project, platform);
    }
}

BuildBar::BuildBar(QWidget *parent, QAction *build)
    : QWidget(parent)
{
    m_platform = new QComboBox(this);
    for (int index = 0; index < s_platformCount; ++index)
    {
        m_platform->insertItem(index, platformName(static_cast<Platform>(index)));
    }

    m_type = new QComboBox(this);
    m_type->insertItem(comboIndexOf(Build::Type::Debug), Build::name(Build::Type::Debug));
    m_type->insertItem(comboIndexOf(Build::Type::Release), Build::name(Build::Type::Release));
    m_type->setToolTip(QStringLiteral("Debug keeps the symbols a debugger needs. Release optimises, which is "
                                      "what a build for players wants."));

    m_directory = new QLineEdit(this);
    m_directory->setPlaceholderText(QStringLiteral("Build directory"));

    QPushButton *browse = new QPushButton(QStringLiteral("Browse..."), this);

    m_reset = new QPushButton(QStringLiteral("Reset"), this);
    m_reset->setToolTip(QStringLiteral("Back to where the chosen platform builds by default."));

    QToolButton *buildButton = new QToolButton(this);
    buildButton->setDefaultAction(build);
    buildButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    m_warning = new QLabel(this);
    m_warning->setWordWrap(true);
    m_warning->hide();

    QHBoxLayout *controls = new QHBoxLayout;
    controls->addWidget(m_platform);
    controls->addWidget(m_type);
    controls->addWidget(m_directory, 1);
    controls->addWidget(browse);
    controls->addWidget(m_reset);
    controls->addWidget(buildButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_warning);
    layout->addLayout(controls);

    connect(m_platform, &QComboBox::currentIndexChanged, this, &BuildBar::showRemembered);
    connect(m_type, &QComboBox::currentIndexChanged, this, &BuildBar::rememberType);
    connect(browse, &QPushButton::clicked, this, &BuildBar::chooseDirectory);
    connect(m_reset, &QPushButton::clicked, this, &BuildBar::resetDirectory);
    connect(m_directory, &QLineEdit::editingFinished, this, &BuildBar::rememberDirectory);
    connect(m_directory, &QLineEdit::textChanged, this, &BuildBar::directoryChanged);
}

void BuildBar::setProjectDirectory(const QString &directory)
{
    m_projectDirectory = directory;
    showRemembered();
}

Platform BuildBar::platform() const
{
    return static_cast<Platform>(m_platform->currentIndex());
}

Build::Type BuildBar::type() const
{
    return static_cast<Build::Type>(m_type->currentIndex());
}

// Whatever follows the token is relative to the project; anything else stands on its own.
QString BuildBar::directory() const
{
    QString typed = m_directory->text().trimmed();
    if (!typed.startsWith(s_projectToken))
    {
        return typed;
    }

    typed = typed.mid(s_projectToken.size());
    while (typed.startsWith(QChar('/')) || typed.startsWith(QChar('\\')))
    {
        typed.remove(0, 1);
    }
    return typed;
}

void BuildBar::remember()
{
    rememberDirectory();
    rememberType();
}

void BuildBar::showRemembered()
{
    QSettings settings;
    const QString type =
        settings.value(typeKey(m_projectDirectory, platform()), Build::name(Build::Type::Debug)).toString();
    const QString directory =
        settings.value(directoryKey(m_projectDirectory, platform()), Build::defaultDirectory(platform())).toString();

    // Showing what was remembered is not a choice, so it stores nothing.
    const QSignalBlocker blocker(m_type);
    m_type->setCurrentIndex(comboIndexOf(Build::typeNamed(type)));
    showDirectory(directory);

    // The text may not have changed, but the platform it is checked against has.
    directoryChanged();
}

void BuildBar::showDirectory(const QString &directory)
{
    m_directory->setText(QDir::isAbsolutePath(directory) ? QDir::toNativeSeparators(directory)
                                                         : s_projectToken + QChar('/') + directory);
}

void BuildBar::directoryChanged()
{
    const QString chosen = directory();
    m_directory->setToolTip(QDir(m_projectDirectory).filePath(chosen));
    m_reset->setEnabled(chosen != Build::defaultDirectory(platform()));

    const QString problem = vitaSpaceProblem();
    m_warning->setText(problem);
    m_warning->setVisible(!problem.isEmpty());
}

void BuildBar::chooseDirectory()
{
    const QDir project(m_projectDirectory);
    const QString chosen =
        QFileDialog::getExistingDirectory(this, QStringLiteral("Build Directory"), project.filePath(directory()));
    if (chosen.isEmpty())
    {
        return;
    }

    // A directory inside the project travels with it, so it is kept relative.
    const QString relative = project.relativeFilePath(chosen);
    showDirectory(relative.startsWith(QStringLiteral("..")) ? chosen : relative);
    rememberDirectory();
}

void BuildBar::resetDirectory()
{
    showDirectory(Build::defaultDirectory(platform()));
    rememberDirectory();
}

void BuildBar::rememberType()
{
    if (m_projectDirectory.isEmpty())
    {
        return;
    }

    QSettings settings;
    settings.setValue(typeKey(m_projectDirectory, platform()), Build::name(type()));
}

void BuildBar::rememberDirectory()
{
    if (m_projectDirectory.isEmpty() || directory().isEmpty())
    {
        return;
    }

    QSettings settings;
    settings.setValue(directoryKey(m_projectDirectory, platform()), directory());
}

// VitaSDK hands vita-pack-vpk every path unquoted, so a space splits an argument.
QString BuildBar::vitaSpaceProblem() const
{
    if (platform() != Platform::Vita)
    {
        return QString();
    }

    QStringList spaced;

    // Its assets are packed straight from here, wherever the build tree is.
    if (m_projectDirectory.contains(QChar(' ')))
    {
        spaced << QStringLiteral("the project's own path");
    }
    if (directory().contains(QChar(' ')))
    {
        spaced << QStringLiteral("the build directory");
    }

    if (spaced.isEmpty())
    {
        return QString();
    }

    return QStringLiteral("A Vita build will compile but fail to be packaged: VitaSDK does not quote the paths it "
                          "packs with, and there is a space in %1.")
        .arg(spaced.join(QStringLiteral(" and in ")));
}
