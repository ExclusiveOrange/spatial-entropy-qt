#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "entropy.hpp"
#include "entropyfast.hpp"

#include <QDir>
#include <QImage>
#include <QFileDialog>
#include <QFileInfo>
#include <QtConcurrent>

#include <ciso646>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, &MainWindow::openImageSucceeded, this, &MainWindow::onOpenImageSuccess);
    connect(this, &MainWindow::openImageFailed, this, &MainWindow::onOpenImageFailure);
    connect(this, &MainWindow::entropyImageReady, this, &MainWindow::onEntropyImageReady);
    connect(this, &MainWindow::imageSaved, this, &MainWindow::onImageSaved);
}

MainWindow::~MainWindow()
{
    delete ui;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::asyncOpenImageFrom(const QString &imageFilename)
{
    ui->wdgButtons->setEnabled(false);
    ui->statusbar->showMessage("Loading " + imageFilename);
    QtConcurrent::run( [=]() {
        if( QImage image(imageFilename); not image.isNull() )
            emit openImageSucceeded(imageFilename, image);
        else
            emit openImageFailed(imageFilename);
    });
}

void MainWindow::onOpenImageSuccess(const QString &imageFilename, const QImage & image)
{
    setInputImage(imageFilename, image);
    ui->statusbar->showMessage("Loaded " + imageFilename, 5000);
    ui->wdgButtons->setEnabled(true);
}

void MainWindow::onOpenImageFailure(const QString & imageFilename)
{
    ui->statusbar->showMessage("Failed to load image: " + imageFilename);
    ui->wdgButtons->setEnabled(true);
}

void MainWindow::setInputImage(const QString &imageFilename, const QImage &image)
{
    ui->btnSaveEntropyImage->setEnabled(false);
    this->inputImageFilename = imageFilename;
    this->inputImage = image;
    entropyImage = QImage();
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
    ui->wdgButtons->setEnabled(false);
    ui->statusbar->showMessage("Calculating entropy...");
    QtConcurrent::run( [=]() {
        QElapsedTimer elapsedTimer;
        elapsedTimer.start();
        QImage entropyImage = EntropyFast::calculateEntropyImageFrom( this->inputImage );
        emit entropyImageReady( entropyImage, elapsedTimer.nsecsElapsed() );
    });
}

void MainWindow::onEntropyImageReady(const QImage &image, qint64 nsecsElapsed)
{
    this->entropyImage = image;
    showEntropyImage();
    ui->statusbar->showMessage(QString("Entropy calculated in %1 milliseconds").arg(static_cast<double>(nsecsElapsed) / 1000000), 5000);
    ui->btnSaveEntropyImage->setEnabled(true);
    ui->wdgButtons->setEnabled(true);
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
QString MainWindow::getSuggestedEntropyImageFilename() const
{
    QFileInfo inputFileInfo( inputImageFilename );
    return inputFileInfo.path() + QDir::separator() + inputFileInfo.completeBaseName() + "-entropy.jpg";
}

void MainWindow::asyncSaveImageTo(const QImage &image, const QString &imageFilename)
{
    ui->wdgButtons->setEnabled(false);
    ui->statusbar->showMessage("Saving entropy image to " + imageFilename);
    QtConcurrent::run( [=]() {
        image.save(imageFilename);
        emit imageSaved(imageFilename);
    });
}

void MainWindow::onImageSaved(const QString &imageFilename)
{
    ui->statusbar->showMessage("Image saved to " + imageFilename, 5000);
    ui->wdgButtons->setEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_btnChooseImage_clicked()
{
    QString imageFilename = QFileDialog::getOpenFileName(
                this, "Choose an image file", QDir::homePath(),
                "Images (*.bmp *.gif *.jpg *.png)");

    if( not imageFilename.isEmpty() )
        asyncOpenImageFrom(imageFilename);
}

void MainWindow::on_btnShowEntropy_clicked()
{
    showEntropyImage();
}

void MainWindow::on_btnShowOriginal_clicked()
{
    showInputImage();
}

void MainWindow::on_btnSaveEntropyImage_clicked()
{
    QString imageFilename = QFileDialog::getSaveFileName(
                this, "Choose a location and new file name",
                getSuggestedEntropyImageFilename(),
                "Images (*.bmp *.gif *.jpg *.png)");

    if( not imageFilename.isEmpty() )
        asyncSaveImageTo( entropyImage, imageFilename );
}
