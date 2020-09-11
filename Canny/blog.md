# 图像基础：Canny算子

以前在入门图像处理的时候，很多基础的东西没怎么特别去看详细的原理什么的，而是直奔各种论文去。许多图像处理的方法也都懂个大概，但是细枝末节的东西都没有考究。现在想着要是再碰到了，就再简单学习一下，梳理一下，记录一下。明白了原理之后再来看处理方法的效果，比起直接拿来用那可舒坦多了。

冈萨雷斯的数字图像处理中就列举了不少检测算子，从最初的Roberts算子，Prewitt算子到Sobel算子，以上三个可以说是最为经典的一阶微分算子了。直接从离散的模板上来看，几个算子的变化可以说的并不大，但也不是一蹴而就的。看似简单的模板背后，很多都有比较严谨的数理证明，如Sobel算子也具有很多值得一看可以理解的东西，这里就不说啦，放2个链接[1](http://blog.sciencenet.cn/home.php?mod=space&uid=425437&do=blog&quickforward=1&id=776050)、[2](http://blog.sciencenet.cn/blog-425437-1139187.html)。

Canny算子是28岁的John Canny在1986年提出的，原始文献为：

> 1986. *[A computational approach to edge detection](http://portal.acm.org/citation.cfm?id=11275)*. IEEE Transactions on Pattern Analysis and Machine Intelligence, vol. 8, 1986, pp. 679-698

即使是现在实现起来并不复杂的Canny算子，要是去看原文，还是感觉东西有点多，20页的论文实在难顶。Canny算子主要就是4个步骤，论文里对每一步为什么这么做都进行了详细的比较分析。本文还是简单的结合每一步的核心思想和C++代码过一遍，主要就是以下几个步骤：

- 高斯滤波
- 计算梯度及其方向
- 非极大值抑制
- 双阈值连接

文末提供C++的处理代码，弄了一个简单QT工程用于学习测试。`QImage`仅用来读图，处理部分完全用原生C++来处理。几个处理函数，单独写在一个`.h`和`.cpp`中，用opencv也可以直接调用里面的接口跑，调用说明自行阅读代码。

24:00 ，老网抑云玩家不请自来，以下以这张图为例，展示效果。

![ori](https://img-blog.csdnimg.cn/20200907210036833.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

## 高斯滤波

图像的噪声对边缘检测影响极大，噪声和边缘同属图像中的高频成分，一旦噪声影响严重，边缘检测的结果势必会不尽人意。如Sobel算子的模板也隐含了去噪的意思，但是想要一个模板又去噪又检测边缘，那肯定不如分开做效果来的好。Canny在这就是用的高斯滤波在平滑去噪，高斯滤波器算是图像处理中的万金油了，哪里都能见到它。Canny在论文里对高斯滤波对于边缘检测的好处有比较详细的探究，虽然我也没仔细看，大家有兴趣可以去看原文。

高斯滤波的显著特点就是权重中心扩散，既能比较好的保留原有的边缘信息，又能抑制噪声，算是一个在不影响实际边缘检测效果前提下很好的去噪选择了。同时模板也比较简单，由于其二维高斯卷积还可以在两个方向分离的特性，优化也比较成熟。

高斯滤波的公式和原理我就不放了，相关的内容很多了，直接贴代码了。

滤波输入是指向图像第一个像素的指针，图像宽、高和滤波核的半径r，以及高斯核的sigma。图像默认是用单通道的灰度图，三通道的拓展也是一样的。

```c++
//直接用vector来存放2维数据，方便理解，typedef了一下
typedef vector<int>         Mat1D;
typedef vector<vector<int>> Mat2D;
//一维高斯核
vector<double> gaussian1D(int r, double sigma){
    int w = 2*r+1;
    vector<double> arr(w);
    double sum = 0;
    for(int i=0;i<w;i++){
        arr[i] = exp(-pow((i-r),2)/(2.0*sigma*sigma));
        sum+=arr[i];
    }
    for(double &a:arr){
        a/=sum;
    }
    return arr;
}

//高斯滤波，分离为X方向和Y方向滤波
//边缘进行对称处理
void _gaussFilter(const uint8_t *bits, Mat2D &dst, int width, int height, int r, double sigma){
    dst.resize(height,Mat1D(width));
    for(int h=0;h<height;h++){
        for(int w=0;w<width;w++){
            int ptr = h*width+w;
            dst[h][w] = bits[ptr];
        }
    }

    Mat2D tmp(dst);
	//生成高斯核
    vector<double> guassArr = gaussian1D(r, sigma);
	//x方向滤波
    double sum;
    for(int h=0;h<height;h++){
        for(int w=0;w<width;w++){
            sum = 0;
            for(int k=-r;k<=r;k++){
                if(w+k>=0 && w+k<width){
                    sum+=guassArr[k+r]*dst[h][w+k];
                }
                else if(w+k<0){
                    sum+=guassArr[k+r]*dst[h][-w-k];
                }
                else{
                    sum+=guassArr[k+r]*dst[h][2*width-w-k-1];
                }
                tmp[h][w] = uint8_t(sum);
            }
        }
    }
	//y方向滤波
    for(int h=0;h<height;h++){
        for(int w=0;w<width;w++){
            sum = 0;
            for(int k=-r;k<=r;k++){
                if(h+k>=0 && h+k<height){
                    sum+=guassArr[k+r]*tmp[h+k][w];
                }
                else if(h+k<0){
                    sum+=guassArr[k+r]*dst[h][-h-k];
                }
                else{
                    sum+=guassArr[k+r]*dst[h][2*height-h-k-1];
                }
                dst[h][w] = uint8_t(sum);
            }
        }
    }
}
```

![guass](https://img-blog.csdnimg.cn/20200907210123778.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

## 计算梯度及其方向

Canny算子的核心思想主要还是在第3步和第4步，这一步其实就是检测一个比较粗略的边缘，或者说是梯度。图像梯度的计算无非来源于邻近像素的差分，这里可以用各种求梯度的算子，最简单的前向差分后向差分，或者几个经典的一阶微分算子都是可以用来计算梯度的。但是反正都能用，不如直接用**sobel**算子来的直接，本身就比较适合用来检测边缘，求解的边缘也比较顺滑，更加适合后续的非极大值抑制。

利用X方向和Y方向的sobel算子分别检测水平和竖直的边缘，然后根据这两个结果求解最终的梯度及其方向即可。

sobel算子：

X方向：$\begin{bmatrix} 1 & 0 & 1\\2&0&-2\\1&0&-1 \end{bmatrix}$，Y方向：$\begin{bmatrix} 1&2&1\\0&0&0\\-1&-2&-1 \end{bmatrix}$。



用上述滤波核滤波分别可以得到$gradX$和$gradY$，最终的梯度幅值为$grad=(gradX^2+gradY^2)^{1/2}$，$\frac{gradY}{gradX}$即可表示方向角的tan值。

那这一步主要就计算了每个像素点的梯度幅值和梯度方向，为后续的非极大值抑制作准备。这部分的代码和在非极大值抑制的代码在下面的部分写在一起了。sobel算子计算得到的边缘如下图所示，可以看到，边缘都比较厚，但是整体边缘在连接上都很顺滑。

![sobel](https://img-blog.csdnimg.cn/20200907210138278.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

## 非极大值抑制

非极大值抑制是Canny算子的核心优化所在，但是要理解起来也不是很难。上面经过sobel算子处理的结果中可以看到，一方面，图像的边缘都比较宽，尤其是那种明显的边缘处；另一方面，在边缘密集处容易出现很多我们不希望的边缘，如下面的字幕处；当然还有这张图中并没有反应出来的噪声问题。这些都是非极大值抑制要解决的问题。

如上面的图，可以想象，我们想要的是前景中的人物和背景分离，假设总存在一条最为突出的边缘，该边缘周围的点则都是一些边缘检测过程中带来的附属品或者噪声等等。那这一步的目标就是从sobel算子所检测出来的边缘里，提取单条明显边缘，抑制周围的一些干扰点。抑制方法也好理解，**对任意一个像素点，沿着该像素点的梯度方向，该点是否是一个局部极大值。如果是则保留该值，否则抑制**。

那如何定义这个局部范围，最简单的当然是像素点周围**8个像素点的邻域范围**，如下图所示。

以左图为例，当前像素点为c，根据上一步得到的梯度方向，为图中的蓝色直线。分别交$g_1,g_2$于$dTmp_1$和$g_3,g_4$于$dTmp2$。直接比较$c,dTmp_1,dTmp_2$三个点的梯度值就行，只有当c是最大值是才保留，否则抑制。即：

$gradC = \begin{cases}gradC,&\quad \text{if } gradC>\max(dTmp_1,dTmp_2)\\0,& \quad \text{else } \end{cases}$

$dTmp_1,dTmp_2$的值可以用插值的方法计算，$dTmp_1$的值就可以由$g_1,g_2$插值得到，$dTmp_1=w_1*g_1+w_2*g_2$，其中，$w_1,w_2$就是上一步得到的在c点在x，y方向上的梯度比值计算而来。而实际上，一共有4种方向，会交在不同的邻域像素之间，需要判别一下。

利用插值在判断当前像素的在梯度方向上的邻近像素值相对而言会准确一点，但是实际上右图中的四种方向可以直接近似到0度、45度、90度和135度。这样可以省去插值的步骤，直接和8邻域的8个像素点的梯度值比较即可。

![y](https://img-blog.csdnimg.cn/20200907210246932.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

以下为sobel算子滤波和非极大值抑制的代码，为了方便，直接忽略了最外围一圈像素的计算。

```c++
//sobel滤波，忽略了周围一圈像素的处理
void _sobelFilter(const Mat2D &src, Mat2D &dst, bool sup=true){
    //sobel核
    double sobelx[9]{1,0,-1,2,0,-2,1,0,-1};
    double sobely[9]{1,2,1,0,0,0,-1,-2,-1};

    //sobel filter
    int H = src.size();
    int W = src[0].size();
    Mat2D sobelVal(H,Mat1D(W));
    Matd2D sobelDirect(H,Matd1D(W));
    int p;
    double sumx,sumy;
    for(int h=1;h<H-1;h++){
        for(int w=1;w<W-1;w++){
            sumx=0,sumy=0;
            for(int i=-1;i<=1;i++){
                for(int j=-1;j<=1;j++){
                    p=(i+1)*3+j+1;
                    sumx+=sobelx[p]*src[i+h][j+w];
                    sumy+=sobely[p]*src[i+h][j+w];
                }
            }
            //梯度幅值
            sobelVal[h][w]=pow(pow(sumx,2)+pow(sumy,2),0.5);
            //梯度y与x比(方向tan)
            sobelDirect[h][w]=sumy/(sumx+0.0001);
        }
    }

    //方便处理两步放一起了，标识是否进行非极大值抑制
    if(!sup){
        dst = sobelVal;
        return;
    }

    // non-maximum suppression 非极大值抑制
    // 根据梯度方向对8邻域像素进行线性插值来判断是否极大值
    int direct;
    double x;
    dst = sobelVal;
    for(int h=1;h<H-1;h++){
        for(int w=1;w<W-1;w++){
            //判断为4种情况中的哪一种，0-45度，45-90度，90-135度，135-180度。
            direct = int(floor(atan(sobelDirect[h][w])/PI*4+8))%4;
            //计算sumy/sumx的绝对值，当x>y时的x，1-x表示线性插值的权重，当y>x时1/x,1-1/x表示线性插值的权重
            x = abs(sobelDirect[h][w]);
            switch (direct) {
            case 0:
                if(sobelVal[h][w]<max(x*sobelVal[h][w+1]+(1-x)*sobelVal[h-1][w+1],x*sobelVal[h][w-1]+(1-x)*sobelVal[h+1][w-1]))
                    dst[h][w]=0;
                break;
            case 1:
                if(sobelVal[h][w]<max(1/x*sobelVal[h-1][w]+(1-1/x)*sobelVal[h-1][w+1],1/x*sobelVal[h+1][w]+(1-1/x)*sobelVal[h+1][w-1]))
                    dst[h][w]=0;
                break;
            case 2:
                if(sobelVal[h][w]<max(1/x*sobelVal[h-1][w]+(1-1/x)*sobelVal[h-1][w-1],1/x*sobelVal[h+1][w]+(1-1/x)*sobelVal[h+1][w+1]))
                    dst[h][w]=0;
                break;
            case 3:
                if(sobelVal[h][w]<max(x*sobelVal[h][w-1]+(1-x)*sobelVal[h-1][w-1],x*sobelVal[h][w+1]+(1-x)*sobelVal[h+1][w+1]))
                    dst[h][w]=0;
                break;
            }
        }
    }
}
```

## 双阈值连接

最后一步，在非极大值抑制处理后的图像上，进行双阈值抑制并将图像二值化从而连接边缘。之前的步骤可以说主要是在像素邻域上对一些非极大值边缘进行抑制，而这一步则是一个更加全局的阈值，来过滤那些噪声，或者一些零碎的小边缘。

双阈值顾名思义有两个阈值$t_1,t_2$，这里假设$t_1$为低阈值，$t_2$为高阈值。所以梯度值大于$t_2$的称为强边缘，若图像为0-1浮点型，则直接设为1，若是uint8型，直接设为255。而对于梯度值在$t_1,t_2$之间的弱边缘，若其周围存在强边缘，则也保留为255，否则抑制为0。因为认为弱边缘周围必然有强边缘支撑，不然这个弱边缘很有可能是噪声等。而对于小于$t_1$的梯度值，直接抑制为0。

进行一遍处理后，结果即为一个0-1二值化的Mask了。

```c++
//双阈值抑制，忽略了周围一圈像素的处理
void _thresholdDeal(const Mat2D &src, Mat2D &dst, double T1, double T2){
    int H = src.size();
    int W = src[0].size();
    dst = src;
    
	//0<T1<T2<1
    int t1=T1*255;
    int t2=T2*255;

    for(int h=1;h<H-1;h++){
        for(int w=1;w<W-1;w++){
            //直接抑制
            if(src[h][w]<t1){
                dst[h][w]=0;
            }
            //弱边缘，判断是否保留
            else if(src[h][w]<t2){
                bool sup = false;
                for(int i=-1;i<=1;i++){
                    for(int j=-1;j<1;j++){
                        if(src[h+i][w+j]>t2) sup=true;
                    }
                }
                if(sup) dst[h][w]=0;
                else dst[h][w]=255;
            }
            else{
                dst[h][w]=255;
            }
        }
    }
}
```

当然，这一步进行阈值连接的方法还可以别的思路。

## 结果

以下图像是Canny算子取不同双阈值的结果，从左到右分别为：

- $t_1=0.1,t_2=0.6$
- $t_1=0.2,t_2=0.6$
- $t_1=0.3,t_2=0.6$

我这里主要展示了低阈值不同情况下，对一些弱边缘的判定。而强边缘若不是一些边缘特别不明显的图像，一般主要轮廓都能包括。

![canny10-60](https://img-blog.csdnimg.cn/20200907210327424.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

![canny20-60](https://img-blog.csdnimg.cn/20200907210342310.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

![canny30-60](https://img-blog.csdnimg.cn/2020090721040021.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

而且，这里其实还有一点问题。

一个是可能是我sobel算子的滤波写的有问题，图像中会有一道摩尔纹，是因为没处理边缘像素点的原因吗。

另一个是最终的结果也差点意思，有些边缘感觉连接的不是很好，不是那么丝滑。

暂留，之后再看，有头绪的小伙伴可以留言分享一下。

## 代码

[Github]()

[CSDN直接下载]()

![code](https://img-blog.csdnimg.cn/20200907210627618.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzE5NDMwNQ==,size_16,color_FFFFFF,t_70#pic_center)

## 参考

[1]、[https://www.cnblogs.com/mmmmc/p/10524640.html](https://www.cnblogs.com/mmmmc/p/10524640.html)

[2]、数字图像处理，冈萨雷斯

