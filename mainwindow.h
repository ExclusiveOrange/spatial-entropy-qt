#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void openImageSucceeded(const QString &imageFilename, const QImage &image);
    void openImageFailed(const QString &imageFilename);
    void entropyImageReady(const QImage &image, qint64 nsecsElapsed);
    void imageSaved(const QString &imageFilename);

private:
    QImage inputImage;
    QImage entropyImage;

    void asyncOpenImageFrom(const QString &imageFilename);
    void onOpenImageSuccess(const QString &imageFilename, const QImage &image);
    void onOpenImageFailure(const QString &imageFilename);
    void setInputImage(const QImage &image);
    void showInputImage();

    void asyncCalculateEntropyImage();
    void onEntropyImageReady(const QImage &image, qint64 nsecsElapsed);
    void showEntropyImage();

    void asyncSaveImageTo(const QImage &image, const QString &imageFilename);
    void onImageSaved(const QString &imageFilename);

private slots:
    void on_btnChooseImage_clicked();
    void on_btnShowEntropy_clicked();
    void on_btnShowOriginal_clicked();
    void on_btnSaveEntropyImage_clicked();

private:
    Ui::MainWindow *ui;
};
