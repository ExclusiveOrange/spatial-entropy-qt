#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QImage>
#include <QFileDialog>
#include <QtConcurrent>

#include <ciso646>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, &MainWindow::openImageSucceeded, this, &MainWindow::onOpenImageSuccess);
    connect(this, &MainWindow::openImageFailed, this, &MainWindow::onOpenImageFailure);
}

MainWindow::~MainWindow()
{
    delete ui;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::asyncOpenImageFrom(const QString &imageFilename)
{
    ui->statusbar->showMessage("Loading " + imageFilename);
    QtConcurrent::run( [=]() {
        QImage image(imageFilename);
        if( image.isNull() )
            emit openImageFailed(imageFilename);
        else
            emit openImageSucceeded(imageFilename, image);
    });
}

void MainWindow::onOpenImageSuccess(const QString &imageFilename, const QImage & image)
{
    setInputImage(image);
    ui->statusbar->showMessage("Loaded " + imageFilename, 5000);
    ui->btnChooseImage->setEnabled(true);
}

void MainWindow::onOpenImageFailure(const QString & imageFilename)
{
    ui->statusbar->showMessage("Failed to load image: " + imageFilename);
    ui->btnChooseImage->setEnabled(true);
}

void MainWindow::setInputImage(const QImage &image)
{
    this->inputImage = image;
    showInputImage();
}

void MainWindow::showInputImage()
{
    ui->btnShowOriginal->setEnabled(false);
    ui->lblImage->setPixmap(QPixmap::fromImage(this->inputImage));
    ui->btnShowEntropy->setEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::asyncCalculateEntropyImage()
{
    ui->statusbar->showMessage("Calculating entropy...");
    QtConcurrent::run( [=]() {
        QImage image;
        // todo: outsource the calculation to a dedicated function, or something
        // todo: emit a signal or something
    });
}

void MainWindow::showEntropyImage()
{
    ui->btnShowEntropy->setEnabled(false);
    if( entropyImage.isNull() )
        asyncCalculateEntropyImage();
    else
    {
        ui->lblImage->setPixmap(QPixmap::fromImage(this->entropyImage));
        ui->btnShowOriginal->setEnabled(true);
    }
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_btnChooseImage_clicked()
{
    QString imageFilename = QFileDialog::getOpenFileName(
                this, "Choose an image file", QDir::homePath(),
                "Images (*.bmp *.gif *.jpg *.png)");

    if( not imageFilename.isEmpty() )
    {
        ui->btnChooseImage->setDisabled(true);
        asyncOpenImageFrom(imageFilename);
    }
}

void MainWindow::on_btnShowEntropy_clicked()
{
    showEntropyImage();
}

void MainWindow::on_btnShowOriginal_clicked()
{
    showInputImage();
}
