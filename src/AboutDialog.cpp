// Copyright (c) 2025. Andrew Kevin Bailey
// This code, firmware, and software is released under the MIT License (http://opensource.org/licenses/MIT).
//
// The MIT License (MIT)
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or significant portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "AboutDialog.h"

#include <QClipboard>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QtGlobal>

#ifndef FILEFINDER_COMPILED_FOR
#define FILEFINDER_COMPILED_FOR "Unknown"
#endif

#ifndef FILEFINDER_COMPILED_ON
#define FILEFINDER_COMPILED_ON "Unknown"
#endif

#ifndef FILEFINDER_BUILD_DATE
#define FILEFINDER_BUILD_DATE __DATE__
#endif

#ifndef FILEFINDER_COMPILED_WITH
#define FILEFINDER_COMPILED_WITH "Unknown"
#endif

namespace {
QString applicationProperty(const char* name, const QString& fallback) {
    const QCoreApplication* app = QCoreApplication::instance();
    if (app == nullptr) {
        return fallback;
    }

    const QString value = app->property(name).toString();
    return value.isEmpty() ? fallback : value;
}

QString appNameAndVersion() {
    const QString appName = QCoreApplication::applicationName().isEmpty()
        ? QStringLiteral("FileFinder")
        : QCoreApplication::applicationName();
    const QString appVersion = QCoreApplication::applicationVersion().isEmpty()
        ? QStringLiteral("Unknown")
        : QCoreApplication::applicationVersion();
    return QStringLiteral("%1 %2").arg(appName, appVersion);
}

QString copyrightText() {
    const QString copyrightDate = applicationProperty("copyright_date", QStringLiteral("Unknown"));
    const QString copyrightOwner = QCoreApplication::organizationName().isEmpty()
        ? QStringLiteral("Andrew Kevin Bailey")
        : QCoreApplication::organizationName();

    return QStringLiteral("Copyright (C) %1 %2").arg(copyrightDate, copyrightOwner);
}

QString homepage() {
    return applicationProperty("homepage", QStringLiteral("Unknown"));
}

QLabel* makeValueLabel(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    return label;
}

void addInfoRow(QGridLayout* layout, const int row, const QString& name, const QString& value, QWidget* parent) {
    auto* nameLabel = new QLabel(name, parent);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(nameLabel, row, 0);
    layout->addWidget(makeValueLabel(value, parent), row, 1);
}
}

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("About %1").arg(QCoreApplication::applicationName()));
    setModal(true);
    resize(430, 280);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(QStringLiteral(":/icons/FileFinder.png")).pixmap(48, 48));
    iconLabel->setFixedSize(52, 52);
    iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    headerLayout->addWidget(iconLabel);

    auto* summaryLayout = new QVBoxLayout();
    summaryLayout->setSpacing(4);
    summaryLayout->addWidget(makeValueLabel(appNameAndVersion(), this));
    summaryLayout->addWidget(makeValueLabel(copyrightText(), this));

    auto* homepageLabel = new QLabel(
        QStringLiteral("Homepage: <a href=\"%1\">%1</a>").arg(homepage()), this);
    homepageLabel->setOpenExternalLinks(true);
    homepageLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    summaryLayout->addWidget(homepageLabel);

    headerLayout->addLayout(summaryLayout, 1);
    mainLayout->addLayout(headerLayout);

    auto* buildGroup = new QGroupBox(QStringLiteral("Build information"), this);
    auto* buildLayout = new QGridLayout(buildGroup);
    buildLayout->setContentsMargins(8, 8, 8, 8);
    buildLayout->setHorizontalSpacing(16);
    buildLayout->setVerticalSpacing(6);
    buildLayout->setColumnStretch(1, 1);

    int row = 0;
    addInfoRow(buildLayout, row++, QStringLiteral("Compiled for:"), QString::fromUtf8(FILEFINDER_COMPILED_FOR), buildGroup);
    addInfoRow(buildLayout, row++, QStringLiteral("Compiled on:"), QString::fromUtf8(FILEFINDER_COMPILED_ON), buildGroup);
    addInfoRow(buildLayout, row++, QStringLiteral("Build date:"), QString::fromUtf8(FILEFINDER_BUILD_DATE), buildGroup);
    addInfoRow(buildLayout, row++, QStringLiteral("Compiled with:"), QString::fromUtf8(FILEFINDER_COMPILED_WITH), buildGroup);
    addInfoRow(buildLayout, row, QStringLiteral("Qt version:"), QString::fromLatin1(qVersion()), buildGroup);

    mainLayout->addWidget(buildGroup);

    auto* buttonLayout = new QHBoxLayout();
    auto* copyButton = new QPushButton(QStringLiteral("Copy to clipboard"), this);
    auto* okButton = new QPushButton(QStringLiteral("OK"), this);
    okButton->setDefault(true);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);

    connect(copyButton, &QPushButton::clicked, this, [] {
        QGuiApplication::clipboard()->setText(AboutDialog::clipboardText());
    });
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    setFixedSize(430, 280);
}

QString AboutDialog::clipboardText() {
    return QStringLiteral(
        "%1\n"
        "%2\n"
        "Homepage: %3\n"
        "Compiled for: %4\n"
        "Compiled on: %5\n"
        "Build date: %6\n"
        "Compiled with: %7\n"
        "Qt version used at run-time: %8"
    ).arg(
        appNameAndVersion(),
        copyrightText(),
        homepage(),
        QString::fromUtf8(FILEFINDER_COMPILED_FOR),
        QString::fromUtf8(FILEFINDER_COMPILED_ON),
        QString::fromUtf8(FILEFINDER_BUILD_DATE),
        QString::fromUtf8(FILEFINDER_COMPILED_WITH),
        QString::fromLatin1(qVersion())
    );
}
