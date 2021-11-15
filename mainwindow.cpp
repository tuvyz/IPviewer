#include "mainwindow.h"
#include "ui_mainwindow.h"



using namespace std;
using namespace cv;





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    
    connect(this, SIGNAL(setCursorSignal(const QCursor)), this, SLOT(setCursorSlot(const QCursor)));
    connect(this, SIGNAL(setButtonTextSignal(QPushButton*, const QString&)), this, SLOT(setButtonTextSlot(QPushButton*, const QString&)));
    connect(this, SIGNAL(setButtonIconSignal(QPushButton*, const QPixmap&)), this, SLOT(setButtonIconSlot(QPushButton*, const QPixmap&)));
    connect(this, SIGNAL(criticalMesageSignal(const QString &, const QString &)), this, SLOT(criticalMesageSlot(const QString &, const QString &)));
    connect(this, SIGNAL(setImage(QPixmap)), ui->videoLabel, SLOT(setPixmap(QPixmap)));
    connect(ui->videoLabel, SIGNAL(wheelSpinsSignal(int)), this, SLOT(wheelSpinsSlot(int)));
    connect(this, SIGNAL(setNotification(const QString&)), ui->notificationEdit, SLOT(setText(const QString&)));
    connect(this, SIGNAL(setCoordinatesLabel(const QString&)), ui->coordinatesLabel, SLOT(setText(const QString&)));
    connect(this, SIGNAL(setFpsLabel(const QString&)), ui->fpsLabel, SLOT(setText(const QString&)));
    
    
    // Изъятие картинок из ресурсов
    QImage qCross = QImage(":/pic/Images/cross.png").convertToFormat(QImage::Format_ARGB32);
    cross = Mat(qCross.height(), qCross.width(), CV_8UC4, const_cast<uchar*>(qCross.bits()), static_cast<size_t>(qCross.bytesPerLine())).clone();
    pixRedCircle.load(":/pic/Images/redCircle.png");
    pixRedSquare.load(":/pic/Images/redSquare.png");
    pixScreen.load(":/pic/Images/screen.png");
    
    emit setButtonIconSignal(ui->writeScreenBtn, pixScreen);
    emit setButtonIconSignal(ui->writeVideoBtn, pixRedCircle);
    
    
    // Изъятие ранее сохранённых IP-адресов из памяти
    QByteArray buf = settings->value(KeySettingsIp).toByteArray();
    QDataStream stream(&buf, QIODevice::ReadOnly);
    stream >> deviceAddresses;
    // Заполнение выкидного виджета этими адресами
    ui->camSelector->setPlaceholderText("Сохранённые IP-адреса");
    for (auto deviceAddress : deviceAddresses)
        ui->camSelector->addItem(QString::fromStdString(deviceAddress.address));

}

MainWindow::~MainWindow()
{
    delete ui;
}





void MainWindow::tryConnection(DeviceAddress tempDeviceAddress) {
    
    emit setCursorSignal(Qt::BusyCursor);
    emit setButtonTextSignal(ui->startBtn, "Отмена");
    
    // Попытка подключения или ожидание сигнала отмены подключения
    shared_ptr<VideoCapture> tempCamDevice(new VideoCapture);
    shared_ptr<mutex> grabMutex(new mutex);
    shared_ptr<bool> isThreadCompleted(new bool(false));
    // Поток пытается подключиться к устройству. Он может существовать и после уничтожения
    // вызвавшей его функции, если статус был изменён на "отключение"
    std::thread openTh([](DeviceAddress tempDeviceAddress, shared_ptr<VideoCapture> tempCamDevice, shared_ptr<bool> isThreadCompleted, shared_ptr<mutex> grabMutex){
        tempCamDevice->open(tempDeviceAddress.getFullAddress());
        grabMutex->lock();
        if (*isThreadCompleted == false)
            *isThreadCompleted = true;
        else {
            grabMutex->unlock();
            return;
        }
        grabMutex->unlock();
    }, tempDeviceAddress, tempCamDevice, isThreadCompleted, grabMutex);
    openTh.detach();
    // Цикл ожидает завершение подключения либо изменение статуса на "отключение"
    while (*isThreadCompleted == false) {
        MyTime::sleep(1);
        if (status == DISCONNECTION) {
            grabMutex->lock();
            if (*isThreadCompleted == false) {
                emit setButtonTextSignal(ui->startBtn, "Запустить поток");
                emit setCursorSignal(Qt::ArrowCursor);
                status = NOT_CONNECTED;
                *isThreadCompleted = true;
                grabMutex->unlock();
                return;
            }
            grabMutex->unlock();
        }
    }


    // Успешное подключение
    if (tempCamDevice->isOpened()) {
        emit setNotification("Подключено");
        camDevice = *tempCamDevice;
        currentDeviceAddress = tempDeviceAddress;
        emit setButtonTextSignal(ui->startBtn, "Остановить поток");
        emit setCursorSignal(Qt::ArrowCursor);
        status = CONNECTED;
        std::thread th(&MainWindow::videoStream, this);
        th.detach();
        
        // Сохранение камеры в память
        if (find_if(deviceAddresses.begin(), deviceAddresses.end(), [this](DeviceAddress deviceAddress){ // Если камера уже есть, то она не должна вноситься
                    return deviceAddress == currentDeviceAddress;
    }) == deviceAddresses.end() && 
                currentDeviceAddress.address.substr(0, 7) == "rtsp://") // проверка на то, что адрес является камерой
        {
            deviceAddresses.push_back(currentDeviceAddress);
            settings->remove(KeySettingsIp);
            QByteArray buf;
            QDataStream stream(&buf, QIODevice::WriteOnly);  
            stream << deviceAddresses;
            settings->setValue(KeySettingsIp, buf);
            ui->camSelector->addItem(QString::fromStdString(currentDeviceAddress.address));
        }
    }
    // Подключение не удалось
    else {
        emit setButtonTextSignal(ui->startBtn, "Запустить поток");
        emit setCursorSignal(Qt::ArrowCursor);
        emit criticalMesageSignal("Ошибка получения видеопотока",
                                  "Убедитесь, что вы ввели правильный и поддерживаемый путь к видеофайлу,"
                                  "<br>или правильный URL-адрес канала RTSP!");
        status = NOT_CONNECTED;
    }
    
}






void MainWindow::videoStream() {
    
    uint8_t zoom = 0;
    
    auto fps = camDevice.get(CAP_PROP_FPS);
    if (fps > 0 && fps < 1000)
        emit setFpsLabel("fps: " + QString::number(fps));
    else {
        emit setFpsLabel("fps:NaN");
        fps = 30;
    }
    
    
    while (status == CONNECTED) {
        
        // Обработчик прокрутки
        if (wheelSpins == 1) {
            zoom *= 2;
            wheelSpins = 0;
        } else if (wheelSpins == -1) {
            zoom /= 2;
            wheelSpins = 0;
        }
        zoom = qBound<uint8_t>(1, zoom, 255);
        

        // Захват изображения
        Mat image;
        camDevice >> image;
        if (image.cols <= 0) {
            emit setNotification("Соединение потеряно");
            break;
        }
        
        
        // Масштабирование изображения
        Mat resizeImage = graphics.resize(image, Size(ui->videoLabel->width(), ui->videoLabel->height()), ui->videoLabel->pos(), zoom, ui->videoLabel->leftButton);
        
        
        // Отрисовка крестов
        if (isBigCrossRequest) {
            MyPoint firstPoint = graphics.OriginalToResizePoint(MyPoint(0, image.rows / 2));
            MyPoint secondPoint = graphics.OriginalToResizePoint(MyPoint(image.cols, image.rows / 2));
            graphics.drawLines(resizeImage, PairPoint_t(MyPoint(firstPoint.x, firstPoint.y - 1), MyPoint(secondPoint.x, secondPoint.y - 1)), Vec3b(0, 0, 0), false, false, 0.2f);
            graphics.drawLines(resizeImage, PairPoint_t(firstPoint, secondPoint), Vec3b(147, 227, 113), false, false, 0.8f);
            graphics.drawLines(resizeImage, PairPoint_t(MyPoint(firstPoint.x, firstPoint.y + 1), MyPoint(secondPoint.x, secondPoint.y + 1)), Vec3b(0, 0, 0), false, false, 0.2f);
            
            firstPoint = graphics.OriginalToResizePoint(MyPoint(image.cols / 2, 0));
            secondPoint = graphics.OriginalToResizePoint(MyPoint(image.cols / 2, image.rows));
            graphics.drawLines(resizeImage, PairPoint_t(MyPoint(firstPoint.x - 1, firstPoint.y), MyPoint(secondPoint.x - 1, secondPoint.y)), Vec3b(0, 0, 0), false, false, 0.2f);
            graphics.drawLines(resizeImage, PairPoint_t(firstPoint, secondPoint), Vec3b(147, 227, 113), false, false, 0.8f);
            graphics.drawLines(resizeImage, PairPoint_t(MyPoint(firstPoint.x + 1, firstPoint.y), MyPoint(secondPoint.x + 1, secondPoint.y)), Vec3b(0, 0, 0), false, false, 0.2f);
        } else if (isCrossRequest)
            graphics.insertPicture(resizeImage, cross, MyPoint(image.cols / 2, image.rows / 2), true, 0.8f);
        
        
        // Сохранение скриншота
        if (isScreenWriteRequest) {
            LOCK;
            isScreenWriteRequest = false;
            UNLOCK;
            QString file;
            int n = 0;
            do
                file = screenWriteDirectory + "/Снимок №" + QString::number(++n) + ".png";
            while (QFileInfo::exists(file));
            QImage qImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
            qImage.rgbSwapped().save(file);
            emit setNotification("Снимок сохранён в " + file);
        }
        
        
        // Запись видео
        if (isVideoWriteRequest) {
            LOCK;
            isVideoWriteRequest = 0;
            UNLOCK;
            
            if (videoWriter.isOpened()) {
                videoWriter.release();
                emit setButtonIconSignal(ui->writeVideoBtn, pixRedCircle);
            } else {
                QString file;
                int n = 0;
                do
                    file = videoWriteDirectory + "/Запись №" + QString::number(++n) + ".mp4";
                while (QFileInfo::exists(file));
                
                videoWriter.open(file.toStdString(), VideoWriter::fourcc('m','p','4','v'), fps, Size(image.cols, image.rows));
                emit setButtonIconSignal(ui->writeVideoBtn, pixRedSquare);
            }
        }
        if (videoWriter.isOpened()) {
            videoWriter.write(image);
            //graphics.insertPicture(resizeImage, redCircle, MyPoint(resizeImage.cols - 25, 25));
        }
        
        
        
        
        
        emit setCoordinatesLabel("x: " + QString::number(int(graphics.ResizeToOriginalPoint(ui->videoLabel->pos()).x)) +
                                      "\n" + "y: " + QString::number(int(graphics.ResizeToOriginalPoint(ui->videoLabel->pos()).y)));
        
        QImage qResizeImage(resizeImage.data, resizeImage.cols, resizeImage.rows, resizeImage.step, QImage::Format_RGB888);
        emit setImage(QPixmap::fromImage(qResizeImage.rgbSwapped()));
        
        
        
    }
    
    videoWriter.release();
    camDevice.release();
    emit setButtonTextSignal(ui->startBtn, "Запустить поток");
    emit setImage(QPixmap(QSize(0, 0)));
    status = NOT_CONNECTED;
}


