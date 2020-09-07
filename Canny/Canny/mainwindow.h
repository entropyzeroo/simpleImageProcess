#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "canny.h"

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
    void on_loadImgBtn_clicked();

    void on_gaussianFilterBtn_clicked();

    void on_restoreBtn_clicked();

    void on_sobelFilterBtn_clicked();

    void on_cannyOperatorBtn_clicked();

    void on_saveImgBtn_clicked();

private:
    Ui::MainWindow *ui;
    QRect screenSize;
    void init();

    QImage img;
    QImage procImg;
};
#endif // MAINWINDOW_H
