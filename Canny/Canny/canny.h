#ifndef Canny_H
#define Canny_H

#include <vector>
using namespace std;

typedef unsigned char       uint8_t;
typedef vector<uint8_t>     m_bits;
typedef vector<int>         Mat1D;
typedef vector<vector<int>> Mat2D;

typedef vector<double>                  Matd1D;
typedef vector<vector<double>>          Matd2D;

/*Canny Operator                            Canny算子接口，返回一维指针
 * @params
 * bits:    1D uint8_t pointer              指向图像第一个像素的指针
 * width:   image width                     图像宽
 * height:  image height                    图像高
 * r:       the radius of guassianFilter    高斯核半径
 * sigma:   the sigma of guassianFilter     高斯核sigma
 * T1:      low threshold                   低阈值
 * T2:      high threshold                  高阈值
 */
void cannyOperator(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma, double T1, double T2);

void gaussFilter(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma);

void sobelFilter(const uint8_t *bits, m_bits &dst, int width, int height, int r, double sigma);

void Mat2Bits(const Mat2D &src, m_bits &bits);

#endif//Canny_H
