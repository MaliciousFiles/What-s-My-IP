#include <QtWidgets>
#include <Qt>

#include <iostream>
#include <fstream>

#include "mainwindow.h"

MainWindow::MainWindow() :
        internalIP(new QLabel(QString::fromStdString(wrapIP(getPrivateIP())), this)),
        externalIP(new QLabel(QString::fromStdString(wrapIP(getPublicIP())), this)) {
    setMinimumSize(350, 150);
    setWindowTitle("What's My IP");

    formatIP(internalIP);
    formatIP(externalIP);

    setupLayout();
}

void MainWindow::setupLayout() {
    auto *layout = new QGridLayout();

    Qt::Alignment alignment = Qt::AlignBottom | Qt::AlignHCenter;
    QFont font = QFont("Hey Comic", 13);

    auto internalLabel = new QLabel("Local IP", internalIP);
    internalLabel->setAlignment(alignment);
    internalLabel->setFont(font);
    layout->addWidget(internalLabel, 0, 0);
    layout->addWidget(internalIP, 1, 0);

    auto externalLabel = new QLabel("External IP", internalIP);
    externalLabel->setAlignment(alignment);
    externalLabel->setFont(font);
    layout->addWidget(externalLabel, 0, 1);
    layout->addWidget(externalIP, 1, 1);

    layout->addItem(new QSpacerItem(0, QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight)), 2, 0);

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);
}

void MainWindow::formatIP(QLabel* label) const {
    label->setFont(QFont("Fragment Core", 20));
    label->setOpenExternalLinks(false);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QObject::connect(label, &QLabel::linkActivated, this, &MainWindow::setClipboard);
}

void MainWindow::setClipboard(const QString& text) {
    if (GetOpenClipboardWindow() != nullptr) return;

    std::cout << "clip\n";

    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(text, QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setText(text, QClipboard::Selection);
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
    std::string outfile = "temp";

    #ifdef Q_OS_WINDOWS
    std::string out = getConsoleOutput(("powershell \"(Get-NetIPAddress | Where-Object {$_.AddressState -eq 'Preferred' -and $_.ValidLifetime -lt '24:00:00'}).IPAddress >> "+outfile+'"').c_str(), outfile.c_str()).substr(2);

    std::size_t idx;
    while ((idx = out.find('\000')) < out.length()) {
        out.erase(idx, 1);
    }

    return out.erase(out.find('\n'), 1);
    #elif Q_OS_UNIX || Q_OS_LINUX
    return getConsoleOutput(("ip route get 1 | awk '{print $NF;exit}' >> "+outfile).c_str(), outfile.c_str());
    #elif Q_OS_DARWIN || Q_OS_MACOS
    return getConsoleOutput(("ipconfig getifaddr en0 >> "+outfile).c_str(), outfile.c_str());
    #endif
}

std::string MainWindow::getPublicIP() {
    std::string outfile = "temp";

    return getConsoleOutput(("curl --silent https://myexternalip.com/raw >> "+outfile).c_str(), outfile.c_str());
}

std::string MainWindow::getConsoleOutput(const char* command, const char* outfile) {
    system(command);

    std::ifstream file(outfile);
    std::stringstream buf;
    buf << file.rdbuf();

    std::string out = buf.str();

    file.close();
    std::filesystem::remove(outfile);

    return out;
}

std::string MainWindow::wrapIP(const std::string& ip) {
    return "<a style='text-decoration:none; color: inherit;' href='" + ip + "'>" + ip + "</a>";
}
