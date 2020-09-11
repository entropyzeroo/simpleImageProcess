## Canny算子

### [blog](https://blog.csdn.net/weixin_43194305/article/details/108456134)

### usage

QT调用

```cpp
QImage img(filename);
img = img.convertToFormat(QImage::Format_Grayscale8);

vector<uint_8> data;
int W=img.width(),H=img.height();
int r = 5;
double sigma = 1,5;
double t1=0.2,t2=0.6;
cannyOperator(img.bits(),data,W,H,r,sigma,t1,t2);

QImage dst(data.data(),W,H,QImage::Format_Grayscale8);
```

OpenCV调用

```cpp
Mat img = imread(filename,CV::IMREAD_GRAYSCALE);

vector<uint_8> data;
int W=img.width(),H=img.height();
int r = 5;
double sigma = 1,5;
double t1=0.2,t2=0.6;
cannyOperator(img.data,data,W,H,r,sigma,t1,t2);

Mat dst(W,H,CV_8UC1,(void*)data.data(),img.step.p);
```

