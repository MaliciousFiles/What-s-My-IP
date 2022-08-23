#include <QtWidgets>
#include <Qt>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>
#include <iostream>
#include <sstream>

#include "mainwindow.h"

MainWindow::MainWindow() :
        internalIP(new QLabel(QString::fromStdString(wrapIP(getPrivateIP())), this)),
        internalLabel(new QLabel("Local IP", this)),
        externalIP(new QLabel(QString::fromStdString(wrapIP(getPublicIP())), this)),
        externalLabel(new QLabel("Public IP", this)),
        instructionLabel(new QLabel("click, L, I: copy local IP\nclick, P, E: copy public IP\nESC: exit", this)),
        copiedLabel(new QLabel("", this)) {
    setMinimumSize(350, 150);
    setWindowTitle("What's My IP");

    formatIP(internalIP);
    formatLabel(internalLabel);

    formatIP(externalIP);
    formatLabel(externalLabel);

    instructionLabel->setFont(QFont("Hey Comic", 6));
    instructionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    instructionLabel->setContentsMargins(2, 0, 0, 0);
    instructionLabel->show();

    copiedLabel->setFont(QFont("Hey Comic", 13));
    copiedLabel->setTextFormat(Qt::RichText);
    setCopiedLabelAlpha(0);
    copiedLabel->show();

    setupLayout();

    auto timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->callOnTimeout(this, &MainWindow::adjustCopiedAlpha);
    timer->start(6); // 1530ms (6*255) to totally disappear
}

void MainWindow::adjustCopiedAlpha() {
    int alpha = getCopiedLabelAlpha();

    if (alpha > 0) {
        setCopiedLabelAlpha(alpha - 1);
    }
}

void MainWindow::setCopiedLabelAlpha(int alpha) {
    std::stringstream stream;
    stream << std::hex << alpha;
    std::string hex = stream.str();
    if (hex.length() == 1) hex = "0" + hex;

    copiedLabel->setText(QString::fromStdString("<font color='#"+hex+"4cdb21'>Copied!</font>"));
}

int MainWindow::getCopiedLabelAlpha() {
    std::regex exp("[a-z0-9]{2}(?=[a-z0-9]{6})");
    const std::string s = copiedLabel->text().toStdString();

    std::smatch match;
    if (std::regex_search(s.begin(), s.end(), match, exp)) {
        int hex;
        std::stringstream stream;

        stream << std::hex << match[0];
        stream >> hex;

        return hex;
    }

    return -1;
}

void MainWindow::setupLayout() {
    auto *layout = new QGridLayout();

    layout->addWidget(internalLabel, 0, 0);
    layout->addWidget(internalIP, 1, 0);

    layout->addWidget(externalLabel, 0, 1);
    layout->addWidget(externalIP, 1, 1);

    layout->addItem(new QSpacerItem(0, QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight)/2), 2, 0);

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);
}

void MainWindow::formatLabel(QLabel* label) {
    label->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    label->setFont(QFont("Hey Comic", 13));
}

void MainWindow::formatIP(QLabel* label) const {
    label->setFont(QFont("Fragment Core", 20));
    label->setOpenExternalLinks(false);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QObject::connect(label, SIGNAL(linkActivated(QString)), this, SLOT(setClipboard(QString)));
}

static bool canClip = true;

void MainWindow::setClipboard(const QString& text) {
    #ifdef Q_OS_WINDOWS
    if (GetOpenClipboardWindow() != nullptr) return;
    #endif

    if (!canClip) return;

    canClip = false;

    auto timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setSingleShot(true);
    timer->callOnTimeout(this, [&] {canClip = true;});
    timer->start(1000); // 1s delay between copying

    adjustCopiedLabelPos();
    setCopiedLabelAlpha(255);

    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(text, QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setText(text, QClipboard::Selection);
    }
}

void MainWindow::adjustCopiedLabelPos() {
    copiedLabel->adjustSize();

    copiedLabel->setGeometry((width()-copiedLabel->width())/2, (height()-copiedLabel->height())/6, copiedLabel->width(), copiedLabel->height());
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    if (event->oldSize().width() == -1 || event->oldSize().height() == -1) return;

    float scale = (float) event->size().width() / (float) event->oldSize().width();
    for (QLabel* label : {internalIP, internalLabel, externalIP, externalLabel, instructionLabel, copiedLabel}) {
        QFont font = label->font();
        font.setPointSizeF(font.pointSizeF() * scale);
        label->setFont(font);
    }

    instructionLabel->adjustSize();

    adjustCopiedLabelPos();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_I:
        case Qt::Key_L:
            setClipboard(QString::fromStdString(getPrivateIP()));
            break;
        case Qt::Key_E:
        case Qt::Key_P:
            setClipboard(QString::fromStdString(getPublicIP()));
            break;
        case Qt::Key_Escape:
            QApplication::quit();
            break;
    }
}

std::string MainWindow::getPrivateIP() {
    #if defined(Q_OS_WINDOWS)
    return getConsoleOutput("powershell \"(Get-NetIPAddress | Where-Object {$_.AddressState -eq 'Preferred' -and $_.ValidLifetime -lt '24:00:00'}).IPAddress\"");
    #elif defined(Q_OS_DARWIN) || defined(Q_OS_MACOS)
    return getConsoleOutput("ipconfig getifaddr en0");
    #elif defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    return getConsoleOutput("ip route get 1 | awk '{print $NF;exit}'");
    #endif
}

std::string MainWindow::getPublicIP() {
    return getConsoleOutput("curl --silent https://myexternalip.com/raw");
}

std::string MainWindow::getConsoleOutput(const char *command) {
    std::array<char, 128> buffer {};

    #ifdef Q_OS_WINDOWS
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
    #else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    #endif


    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    std::string res;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        res += buffer.data();
    }

    return res.contains('\n') ? res.erase(res.find('\n'), 1) : res;
}

std::string MainWindow::wrapIP(const std::string& ip) {
    return "<a style='text-decoration: none; color: inherit;' href='" + ip + "'>" + ip + "</a>";
}
