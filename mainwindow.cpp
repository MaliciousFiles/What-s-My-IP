#include <QtWidgets>
#include <Qt>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <regex>

#include "mainwindow.h"

MainWindow::MainWindow() :
        internalIP(new QLabel(QString::fromStdString(wrapIP(getPrivateIP())), this)),
        internalLabel(new QLabel("Local IP", this)),
        externalIP(new QLabel(QString::fromStdString(wrapIP(getPublicIP())), this)),
        externalLabel(new QLabel("Public IP", this)) {
    setMinimumSize(350, 150);
    setWindowTitle("What's My IP");

    formatIP(internalIP);
    formatLabel(internalLabel);

    formatIP(externalIP);
    formatLabel(externalLabel);

    setupLayout();
}

void MainWindow::setupLayout() {
    auto *layout = new QGridLayout();

    layout->addWidget(internalLabel, 0, 0);
    layout->addWidget(internalIP, 1, 0);

    layout->addWidget(externalLabel, 0, 1);
    layout->addWidget(externalIP, 1, 1);

    layout->addItem(new QSpacerItem(0, QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight)), 2, 0);

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

    QObject::connect(label, &QLabel::linkActivated, this, &MainWindow::setClipboard);
}

void MainWindow::setClipboard(const QString& text) {
    #ifdef Q_OS_WINDOWS
    if (GetOpenClipboardWindow() != nullptr) return;
    #endif

    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(text, QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setText(text, QClipboard::Selection);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    if (event->oldSize().width() == -1 || event->oldSize().height() == -1) return;

    float scale = (float) event->size().width() / (float) event->oldSize().width();
    for (QLabel* label : {internalIP, internalLabel, externalIP, externalLabel}) {
        QFont font = label->font();
        font.setPointSizeF(font.pointSizeF() * scale);
        label->setFont(font);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_I:
        case Qt::Key_L:
            setClipboard(QString::fromStdString(getPrivateIP()));
            QApplication::quit();
            break;
        case Qt::Key_E:
        case Qt::Key_P:
            setClipboard(QString::fromStdString(getPublicIP()));
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

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    std::string res;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        res += buffer.data();
    }
    return res;
}

std::string MainWindow::wrapIP(const std::string& ip) {
//    std::regex exp("\\.(?=[0-9])");
//    std::string display = std::regex_replace(ip, exp, "<b style='font-size: 30;'>.</b>");

    return "<a style='text-decoration: none; color: inherit;' href='" + ip + "'>" + ip + "</a>";
}
