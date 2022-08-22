#include <QNetworkAccessManager>
#include <QLabel>
#include <QMainWindow>
#include <QFont>
#pragma once

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QLabel* internalIP;
    QLabel* externalIP;

    static std::string getPrivateIP();
    static std::string getPublicIP();
    static std::string wrapIP(const std::string& ip);

    void setupLayout();
    void formatIP(QLabel *label) const;

    static std::string getConsoleOutput(const char* command, const char* outfile);

private slots:
    static void setClipboard(const QString& text);
};
