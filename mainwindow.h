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
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* internalIP;
    QLabel* internalLabel;
    QLabel* externalIP;
    QLabel* externalLabel;
    QLabel* instructionLabel;
    QLabel* copiedLabel;

    static std::string getPrivateIP();
    static std::string getPublicIP();
    static std::string wrapIP(const std::string& ip);
    void setupLayout();

    void formatIP(QLabel *label) const;
    static void formatLabel(QLabel *label);
    void setCopiedLabelAlpha(int alpha);
    int getCopiedLabelAlpha();

    static std::string getConsoleOutput(const char *command);

private slots:
    void setClipboard(const QString& text);
    void onFrame();
};
