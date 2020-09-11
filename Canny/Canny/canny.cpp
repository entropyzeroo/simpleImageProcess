#include "canny.h"
#include <algorithm>

#define PI 3.1415926

#define BYTE_ALIGN(w) (((w)+31)>>5)<<2

#include <iostream>

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
    Bits2Mat(bits, dst,width,height);

    Mat2D tmp(dst);

    vector<double> guassArr = gaussian1D(r, sigma);

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

//sobel滤波，忽略了周围一圈像素的处理
void _sobelFilter(const Mat2D &src, Mat2D &dst, bool sup=true){
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
//双阈值抑制，忽略了周围一圈像素的处理
void _thresholdDeal(const Mat2D &src, Mat2D &dst, double T1, double T2){
    int H = src.size();
    int W = src[0].size();
    dst = src;

    int t1=T1*255;
    int t2=T2*255;

    for(int h=1;h<H-1;h++){
        for(int w=1;w<W-1;w++){
            if(src[h][w]<t1){
                dst[h][w]=0;
            }
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


void gaussFilter(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma){
    Mat2D guassImg;
    _gaussFilter(bits,guassImg,width,height,r,sigma);

    Mat2Bits(guassImg,dst);
}
void sobelFilter(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma){
    Mat2D guassImg;
    _gaussFilter(bits,guassImg,width,height,r,sigma);

    Mat2D sobelResult;
    _sobelFilter(guassImg,sobelResult,false);

    Mat2Bits(sobelResult, dst);
}
void cannyOperator(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma, double T1, double T2){
    Mat2D guassImg(height, Mat1D(width));
    _gaussFilter(bits,guassImg,width,height,r,sigma);

    Mat2D sobelResult;
    _sobelFilter(guassImg,sobelResult);

    Mat2D result;
    _thresholdDeal(sobelResult,result,T1,T2);

    Mat2Bits(result, dst);
}
void Mat2Bits(const Mat2D &src, m_bits &bits){
    int height = src.size();
    int width = src[0].size();
    //4-byte-align
    int W = BYTE_ALIGN(width*8);

    bits.resize(height*W);
    for(int h=0;h<height;h++){
        for(int w=0;w<width;w++){
            bits[h*W+w]=src[h][w];
        }
    }
}
void Bits2Mat(const uint8_t *bits, Mat2D &dst, int width, int height){
    //4-byte-align
    int W = BYTE_ALIGN(width*8);

    dst.resize(height,Mat1D(width));
    for(int h=0;h<height;h++){
        for(int w=0;w<width;w++){
            dst[h][w] = bits[h*W+w];
        }
    }
}
