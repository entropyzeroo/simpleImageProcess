#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QString"
#include "QStandardPaths"
#include "QApplication"
#include "QDesktopWidget"
#include <QDesktopServices>
#include <QUrl>
#include <QTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init(){
    ui->PicBox->setVisible(false);
    screenSize = QApplication::desktop()->screenGeometry();

    setGeometry(screenSize.width()/2,screenSize.height()/2,400,250);

    ui->slider->setRange(0,100);
    ui->slider->setSingleStep(1);
    ui->slider->setMaxValue(60);
    ui->slider->setMinValue(20);
    ui->slider->setLabel("T1 & T2");

    QPushButton *urlBtn = new QPushButton;
    urlBtn->setCursor(Qt::PointingHandCursor);
    urlBtn->setFlat(true);
    urlBtn->setText("By -> https://blog.csdn.net/weixin_43194305");
    urlBtn->setFixedWidth(300);
    urlBtn->setStyleSheet("QPushButton:hover{color:darkCyan}");
    connect(urlBtn,&QPushButton::clicked,this,[=](){QDesktopServices::openUrl(QUrl(QLatin1String("https://blog.csdn.net/weixin_43194305")));});
    ui->statusBar->addPermanentWidget(urlBtn);
}


void MainWindow::on_loadImgBtn_clicked()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(QStringLiteral("Image"));
    fileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    fileDialog->setNameFilter(tr("Image(*.png;*.jpg;*.jpeg)"));
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setViewMode(QFileDialog::Detail);
    if (fileDialog->exec()){
        auto filename = fileDialog->selectedFiles();
        img.load(filename[0]);
        img = img.convertToFormat(QImage::Format_Grayscale8);
        procImg = img.copy();
        ui->PicBox->setVisible(true);
        ui->PicBox->setMode(PictureBox::AUTO_ZOOM);
        ui->PicBox->setImage(img);
        int W = img.width()+110;
        int H = img.height()+ui->statusBar->height();
        int m_left = max(30,(screenSize.width()-W)/2);
        int m_right = max(30,(screenSize.height()-H)/2);
        setGeometry(m_left,m_right,W,H);
    }
}

void MainWindow::on_saveImgBtn_clicked()
{
    QString saveName = QFileDialog::getSaveFileName(this,"SaveImage","result.png");
    if(!saveName.isNull()){
        procImg.save(saveName,"png");
    }
}

void MainWindow::on_restoreBtn_clicked()
{
    ui->PicBox->setImage(img);
}

void MainWindow::on_gaussianFilterBtn_clicked()
{
    int W=img.width();
    int H=img.height();

    m_bits dst;
    QTime time;
    time.start();
    gaussFilter(img.bits(),dst,W,H,5,1);
    ui->statusBar->showMessage(QString("Running time: %1 ms").arg(time.elapsed()/1000.0));

    QImage res(dst.data(),W,H,QImage::Format_Grayscale8);
    procImg = res.copy();
    ui->PicBox->setImage(res);
}

void MainWindow::on_sobelFilterBtn_clicked()
{
    int W=img.width();
    int H=img.height();

    m_bits dst;
    QTime time;
    time.start();
    sobelFilter(img.bits(),dst,W,H,5,1.5);
    ui->statusBar->showMessage(QString("Running time: %1 ms").arg(time.elapsed()/1000.0));

    QImage res(dst.data(),W,H,QImage::Format_Grayscale8);
    procImg = res.copy();
    ui->PicBox->setImage(res);
}

void MainWindow::on_cannyOperatorBtn_clicked()
{
    int W=img.width();
    int H=img.height();

    double t1 = ui->slider->minValue()/100.0;
    double t2 = ui->slider->maxValue()/100.0;

    m_bits dst;
    QTime time;
    time.start();
    cannyOperator(img.bits(),dst,W,H,5,1.5,t1,t2);
    ui->statusBar->showMessage(QString("Running time: %1 ms").arg(time.elapsed()/1000.0));

    QImage res(dst.data(),W,H,QImage::Format_Grayscale8);
    procImg = res.copy();
    ui->PicBox->setImage(res);
}


