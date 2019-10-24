#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private:
    QImage inputImage;
    QImage entropyImage;

    void asyncOpenImageFrom(const QString &imageFilename);
    void onOpenImageSuccess(const QString &imageFilename, const QImage &image);
    void onOpenImageFailure(const QString &imageFilename);
    void setInputImage(const QImage &image);
    void showInputImage();

    void asyncCalculateEntropyImage();
    void showEntropyImage();

private slots:
    void on_btnChooseImage_clicked();

    void on_btnShowEntropy_clicked();

    void on_btnShowOriginal_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
