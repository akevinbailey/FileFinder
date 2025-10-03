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

// ReSharper disable CppDFAMemoryLeak
#include "MainWindow.h"
#include "FileSearch.h"

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    resize(1280, 600);
    buildUi();
    connectSignals();

    loadSettings(); // prefill the last-used directory
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* layout = new QGridLayout(central);
    layout->setContentsMargins(12,12,12,12);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    auto* lblHeader = new QLabel("File Name:", central);
    leHeaderFile_ = new QLineEdit(central);
    leHeaderFile_->setPlaceholderText("e.g., my-header.h");

    auto* lblSearchDir = new QLabel("Search Directory:", central);
    leSearchDir_ = new QLineEdit(central);
    leSearchDir_->setPlaceholderText("Choose a directory or type one...");
    btnBrowse_ = new QPushButton("Browse…", central);

    btnSearch_ = new QPushButton("Search", central);

    auto* lblFound = new QLabel("Found:", central);
    teFound_ = new QTextEdit(central);
    teFound_->setReadOnly(true);

    int r = 0;
    // Row 1
    layout->addWidget(lblHeader,      r, 0);
    layout->addWidget(leHeaderFile_,  r, 1, 1, 2);

    ++r;
    // Row 2
    layout->addWidget(lblSearchDir,   r, 0);
    layout->addWidget(leSearchDir_,   r, 1);
    layout->addWidget(btnBrowse_,     r, 2);

    ++r;
    // Row 3
    layout->addWidget(btnSearch_,     r, 2);

    ++r;
    // Row 4
    layout->addWidget(lblFound,       r, 0, 1, 3);

    ++r;
    // Row 5
    layout->addWidget(teFound_,       r, 0, 1, 3);

    setCentralWidget(central);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignRight);
    statusBar()->setStyleSheet("QStatusBar::item { border: none; }"); // Prevents the seperator bar from showing
    statusBar()->addPermanentWidget(statusLabel_, 1);
    statusBar()->setContentsMargins(5,0, 0, 2);
    statusLabel_->setText("Ready");
}

void MainWindow::connectSignals() {
    connect(btnBrowse_, &QPushButton::clicked, this, &MainWindow::onBrowse);
    connect(btnSearch_, &QPushButton::clicked, this, &MainWindow::onSearch);
}

void MainWindow::onBrowse() {
    const QString dir = QFileDialog::getExistingDirectory(
        this, "Select Search Directory", QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (!dir.isEmpty()) {
        leSearchDir_->setText(dir);
    }
}

void MainWindow::loadSettings() const {
    const QSettings s;
    QString dir = s.value("lastSearchDir").toString();
    if (dir.isEmpty()) dir = QDir::homePath();
    leSearchDir_->setText(dir);
}

void MainWindow::saveLastDir(const QString& dir) {
    QSettings s;
    s.setValue("lastSearchDir", dir);
}

void MainWindow::onSearch() {
    const QString headerName = leHeaderFile_->text().trimmed();
    const QString searchDir  = leSearchDir_->text().trimmed();

    statusLabel_->setText("Searching…");
    statusLabel_->repaint();

    if (headerName.isEmpty()) {
        QMessageBox::warning(this, "Missing Header File",
                             "Please enter a header file name (e.g., my-header.h).");
        return;
    }
    if (searchDir.isEmpty() || !QDir(searchDir).exists()) {
        QMessageBox::warning(this, "Invalid Directory",
                             "Please provide an existing search directory.");
        return;
    }

    // Load last searched directory
    saveLastDir(searchDir);

    teFound_->clear();

    const auto matches = FileSearch::findFiles(searchDir, headerName);

    if (matches.isEmpty()) {
        teFound_->setPlainText("No matches found.");
        statusLabel_->setText("0 matches");
        return;
    }

    QString out;
    out.reserve(matches.size() * 64);
    for (const auto& path : matches) {
        out.append(path).append('\n');
    }
    teFound_->setPlainText(out);
    if (matches.size() > 1) {
        statusLabel_->setText(QString::number(matches.size()) + " matches");
    }
    else {
        statusLabel_->setText("1 match");
    }
}