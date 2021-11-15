#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "opencv2/opencv.hpp"
#include "graphics.h"
#include <QSettings>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    
    void closeEvent(QCloseEvent *) {
        status = DISCONNECTION;
    }
    void on_startBtn_clicked();
    
    void setButtonTextSlot(QPushButton *button, const QString &text);
    void setButtonIconSlot(QPushButton *button, const QPixmap &icon);
    
    void setCursorSlot(const QCursor &cursor);
    void criticalMesageSlot(const QString &title, const QString &text);
    void wheelSpinsSlot(int upOrDown) {
        wheelSpins = upOrDown;
    }
    
    void on_camSelector_currentIndexChanged(int index);

    
    void on_isBigCrossBtn_clicked();
    void on_isCrossBtn_clicked();
    void on_noCrossBtn_clicked();
    
    void on_writeVideoBtn_clicked();
    void on_writeScreenBtn_clicked();
    
private:
    Ui::MainWindow *ui;
    
    
    enum statuses {
        NOT_CONNECTED,
        TRY_CONNECTION,
        CONNECTED,
        DISCONNECTION
    };
    int status = false;
    
    
    
    QSettings *settings = new QSettings("Strela", "IPviewer"); // Инициализация памяти
    QString KeySettingsIp = "ContainerforIpAddressesOfCameras";
    
    
    
    
    struct DeviceAddress
    {
        DeviceAddress() {}
        DeviceAddress(string address, string login = "", string password = "") {
            if (address.find('@') == string::npos) { // Если адрес не введёт уже с паролем, встроенным в адрес
                this->address = address;
                this->login = login;
                this->password = password;
            } else {
                this->address = "rtsp://";
                this->login = "";
                this->password = "";
                int pos = string("rtsp://").size() - 1;
                while (address[++pos] != ':')
                    this->login.push_back(address[pos]);
                while (address[++pos] != '@')
                    this->password.push_back(address[pos]);
                while (++pos < int(address.size()))
                    this->address.push_back(address[pos]);               
            }
        }
        string address;
        string login;
        string password;
        string getFullAddress() {
            string fullAddress = address;
            string logingAndPassword = login + ':' + password + '@';
            for (int i = logingAndPassword.size() - 1; i >= 0; i--)
                fullAddress.insert(fullAddress.begin() + string("rtsp://").size(), logingAndPassword[i]);
            return fullAddress;
        }
        bool operator == (const DeviceAddress &deviceAddress) {
            return (deviceAddress.address == address &&
                    deviceAddress.login == login &&
                    deviceAddress.password == password);
        }
        void operator = (const DeviceAddress &deviceAddress) {
            address = deviceAddress.address;
            login = deviceAddress.login;
            password = deviceAddress.password;
        }
    };
    QVector<DeviceAddress> deviceAddresses;
    DeviceAddress currentDeviceAddress;
    friend QDataStream &operator << (QDataStream &out, const MainWindow::DeviceAddress &deviceAddress);
    friend QDataStream &operator >> (QDataStream &in, MainWindow::DeviceAddress &deviceAddress);
    
    

    
    cv::VideoCapture camDevice;
    
    Graphics graphics;
    
    int wheelSpins = 0;
    
    bool isBigCrossRequest = false;
    bool isCrossRequest = false;
    bool isScreenWriteRequest = false;
    bool isVideoWriteRequest = false;
    
    QString screenWriteDirectory = "D:";
    QString videoWriteDirectory = "D:";
    
    cv::VideoWriter videoWriter;
    
    cv::Mat cross;
    QPixmap pixRedCircle;
    QPixmap pixRedSquare;
    QPixmap pixScreen;

    void tryConnection(DeviceAddress tempDeviceAddress);
    
    void videoStream();
    
signals:
    
    void setCursorSignal(const QCursor&);
    void setButtonTextSignal(QPushButton*, const QString&);
    void setButtonIconSignal(QPushButton*, const QPixmap &);
    void criticalMesageSignal(const QString &title, const QString &text);
    void setImage(QPixmap);
    void setNotification(const QString&);
    void setCoordinatesLabel(const QString&);
    void setFpsLabel(const QString&);
    
};





inline QDataStream &operator << (QDataStream &out, const MainWindow::DeviceAddress &deviceAddress) {
    out << QString::fromStdString(deviceAddress.address)
        << QString::fromStdString(deviceAddress.login)
        << QString::fromStdString(deviceAddress.password);
    return out;
}
inline QDataStream &operator >> (QDataStream &in, MainWindow::DeviceAddress &deviceAddress) {
    QString buf[3];
    in >> buf[0] >> buf[1] >> buf[2];
    deviceAddress.address = buf[0].toStdString();
    deviceAddress.login = buf[1].toStdString();
    deviceAddress.password = buf[2].toStdString();
    return in;
}






#endif // MAINWINDOW_H
