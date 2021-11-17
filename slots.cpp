#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;





// Установить текст на кнопке
void MainWindow::setButtonTextSlot(QPushButton *button, const QString &text) {
    button->setText(text);
}

// Установить картинку на кнопке
void MainWindow::setButtonIconSlot(QPushButton *button, const QPixmap &icon) {
    button->setIcon(icon);
}

void MainWindow::setCursorSlot(const QCursor &cursor)
{
    setCursor(cursor);
}

void MainWindow::criticalMesageSlot(const QString &title, const QString &text)
{
    QMessageBox::critical(this, title, text);
}

// Выбор адреса из предыдущих успешно подключенных
void MainWindow::on_camSelector_currentIndexChanged(int index)
{
    ui->loginEdit->setText(QString::fromStdString(deviceAddresses[index].login));
    ui->passwordEdit->setText(QString::fromStdString(deviceAddresses[index].password));
    ui->addressEdit->setText(QString::fromStdString(deviceAddresses[index].address));
    //ui->camSelector->setCurrentIndex(-1);
}





// Кнопка запуска потока
void MainWindow::on_startBtn_clicked()
{
    switch (status) {
    case NOT_CONNECTED:
    {
        emit setNotification("Подключение...");
        status = TRY_CONNECTION;
        
        std::thread th(&MainWindow::tryConnection, this, DeviceAddress(ui->addressEdit->text().toStdString(),
                                                                       ui->loginEdit->text().toStdString(),
                                                                       ui->passwordEdit->text().toStdString()));
        th.detach();
        break;
    }
    case TRY_CONNECTION:
    {
        emit setNotification("Подключение отменено");
        status = DISCONNECTION;
        break;
    }
    case CONNECTED:
    {
        emit setNotification("Устройство отключено");
        status = DISCONNECTION;
        break;
    }
    default:
        break;
    }
}



// Кнопки крестов и их отключение
void MainWindow::on_isBigCrossBtn_clicked()
{
    ui->isBigCrossBtn->setEnabled(false);
    ui->isCrossBtn->setEnabled(true);
    ui->noCrossBtn->setEnabled(true);
    isBigCrossRequest = true;
    isCrossRequest = false;
}
void MainWindow::on_isCrossBtn_clicked()
{
    ui->isBigCrossBtn->setEnabled(true);
    ui->isCrossBtn->setEnabled(false);
    ui->noCrossBtn->setEnabled(true);
    isBigCrossRequest = false;
    isCrossRequest = true;
}
void MainWindow::on_noCrossBtn_clicked()
{
    ui->isBigCrossBtn->setEnabled(true);
    ui->isCrossBtn->setEnabled(true);
    ui->noCrossBtn->setEnabled(false);
    isBigCrossRequest = false;
    isCrossRequest = false;
}







// Запись видео
void MainWindow::on_writeVideoBtn_clicked()
{
    LOCK;
    isVideoWriteRequest = true;
    UNLOCK;
}
// Снимок
void MainWindow::on_writeScreenBtn_clicked()
{
    LOCK;
    isScreenWriteRequest = true;
    UNLOCK;
}

// Выбор места сохранения видео
void MainWindow::on_writeVideoPath_clicked()
{
    videoWriteDirectory = QFileDialog::getExistingDirectory(ui->writeVideoPath,
                                                            "Выбор папки сохранения видео",
                                                            videoWriteDirectory);
    if (videoWriteDirectory.isEmpty())
        videoWriteDirectory = QDir::currentPath();
    emit setNotification("Видео будут сохраняться в " + videoWriteDirectory);
    settings->setValue(keyVideoWriteDir, videoWriteDirectory); // Сохранение настроек приложения
}
// Выбор места сохранения снимков
void MainWindow::on_writeScreenPath_clicked()
{
    screenWriteDirectory = QFileDialog::getExistingDirectory(ui->writeScreenPath, 
                                                             "Выбор папки сохранения снимков",
                                                             screenWriteDirectory);
    if (screenWriteDirectory.isEmpty())
        screenWriteDirectory = QDir::currentPath();
    emit setNotification("Снимки будут сохраняться в " + screenWriteDirectory);
    settings->setValue(keyScreenWriteDir, screenWriteDirectory); // Сохранение настроек приложения
}





// Фильтр событий (нужен для меню сохранённых камер)
bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    if (object == ui->camSelector->view()->viewport())
        if (event->type() == QEvent::MouseButtonRelease)
            if (static_cast<QMouseEvent*>(event)->button() == Qt::RightButton)
                return true;
    return false;
}



// Контекстное меню выборщика сохранённых камер
void MainWindow::camSelectorContextMenu(QPoint pos) {
    QAbstractItemView* view = ui->camSelector->view();
    QAction *deleteAction = new QAction("Забыть");
    deleteAction->setData(view->indexAt(pos).column()); // В данные записывается индекс ячейки
    QMenu menu;
    menu.addAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(camSelectorDeleteItem()));
    menu.exec(view->mapToGlobal(pos));
}
// Слот удаления адреса камеры по сигналу из контекстного меню
void MainWindow::camSelectorDeleteItem() {
    QAction *action = qobject_cast<QAction*>(sender());
    int indexItem = action->data().toInt();
    
    ui->camSelector->removeItem(indexItem);
    deviceAddresses.erase(begin(deviceAddresses) + indexItem);
    if (deviceAddresses.isEmpty())
        settings->remove(keyIpAddresses);
    else {
        QByteArray buf;
        QDataStream stream(&buf, QIODevice::WriteOnly);  
        stream << deviceAddresses;
        settings->setValue(keyIpAddresses, buf);
    }
    
    delete action;
}
