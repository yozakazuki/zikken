#include <iostream>
#include <opencv2/opencv.hpp>
#include "Labeling.h"
using namespace std;
using namespace cv;

void point();
void hide(int y);
void line();
bool isFound();
double getAngle();
int  rand_num(int split_num);

int min_x;
int max_x;
int min_y;
int max_y;
int center_x;
int under_y;
int position_x;
int intersection_x;
int intersection_y;
Mat image;
Point test;
double angle;
RegionInfoBS *ri;

int main( int argc, char **argv ) {
    String fileName = "test5.jpg";
    Mat    grayImage;
    Mat    binImage;
    
    image = imread(fileName);
    cvtColor(image, grayImage, CV_BGR2GRAY);
    threshold(grayImage, binImage, 0.0, 255.0, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
    
    Mat label(binImage.size(), CV_16SC1);
    LabelingBS labeling;
    labeling.Exec(binImage.data, (short *)label.data, binImage.cols, binImage.rows, false, 0);
    int labelingSize = labeling.GetNumOfResultRegions();
    srand((unsigned int)time(NULL));
    int rand_labelingNum = rand() % labelingSize;
    cout << "labelingNumber = " << rand_labelingNum << endl;
    ri = labeling.GetResultRegionInfo(rand_labelingNum);
    
    // ラベルされた領域をひとつずつ描画
    for( int i = 0; i < labelingSize; i++) {
        Mat labelArea;
        compare(label, i + 1, labelArea, CV_CMP_EQ);
        bool first_x = true;
        bool first_y = true;
        max_x = 0;
        max_y = 0;
        
        for(int y = 0; y < labelArea.rows; y++)  {
            for(int x = 0; x < labelArea.cols; x++) {
                unsigned char pixel_value = labelArea.at<unsigned char>(y,x);
                if (pixel_value == 255) {
                    if (first_x) {
                        min_x = x;
                        first_x = false;
                    }
                    if (x < min_x) {
                        min_x = x;
                    }
                    if (max_x < x) {
                        max_x = x;
                    }
                    
                    if (first_y) {
                        min_y = y;
                        first_y = false;
                    }
                    if (y < min_y) {
                        min_y = y;
                    }
                    if (max_y < y) {
                        max_y = y;
                    }
                }
            }
        }
        if (i == rand_labelingNum) {
            point();
            line();
            imshow("labelArea", labelArea);
        }
    }
    imshow("image", image);
    cout << "isFound = " << isFound() << endl; // 1の場合ポリゴンと交わる
    waitKey();
}

void point() {
    // 凹のみ対応
    int y_dif = max_y - min_y;
    center_x  = (min_x + max_x) / 2;
    under_y   = min_y - y_dif;
    int top_y = under_y - y_dif;
    
    circle(image, Point(min_x, min_y), 2, Scalar(0,255,0), -1); // 円弧の始点と終点 赤
    circle(image, Point(max_x, min_y), 2, Scalar(0,255,0), -1);
    
    circle(image, Point(center_x, min_y), 2, Scalar(0,255,0), -1); // 円弧の一番上と下 緑
    circle(image, Point(center_x, max_y), 2, Scalar(0,255,0), -1);
    
    hide(top_y);
}

void hide(int top_y) {
    int    split_num = 6; // 分割数
    int    x_dif     = (max_x - min_x) / split_num;
    
    int rand_num = rand() % split_num;
    
    for (int i = 0; i < split_num; i++) {
        int split_x        = (x_dif * i) + min_x;
        int next_split_x   = (x_dif * (i + 1)) + min_x;
        int center_split_x = (split_x + next_split_x) / 2;
        rectangle(image, Point(split_x, top_y), Point(next_split_x, under_y), Scalar(0,0,255), 1, 8);
        if (rand_num == i) {
            position_x         = (center_x + center_split_x) / 2;
            circle(image, Point(position_x, under_y), 2, Scalar(255,0,0), -1);
            circle(image, Point(center_split_x, top_y), 2, Scalar(255,0,0), -1);
            line(image, Point(position_x, under_y), Point(center_split_x, top_y), Scalar(255,0,0), 1, 8);
            
            angle = atan2(under_y - top_y, center_split_x - position_x) * 180 / M_PI;
            cout << "angle = " << angle << endl;
        }
    }
}

void line() {
    int cols = image.cols;
    int rows = image.rows;
    int point_x = cols;
    line(image, Point(cols, rows), Point(position_x, under_y), Scalar(0,255,0), 1, 8);
    
    line(image, Point(min_x, min_y), Point(center_x, max_y), Scalar(0,255,0), 1, 8);
    line(image, Point(max_x, min_y), Point(center_x, max_y), Scalar(0,255,0), 1, 8);
    
    if (point_x < center_x) {
        int area1 = ((min_x - center_x) * (rows - max_y) - (min_y - max_y) * (0 - center_x)) / 2;
        int area2 = ((min_x - center_x) * (max_y - under_y) - (min_y - max_y) * (center_x - position_x)) / 2;
        
        intersection_x = position_x * area1 / (area1 + area2);
        intersection_y = rows + (under_y - rows) * area1 / (area1 + area2);
        Point intersection(intersection_x, intersection_y);
        
        circle(image, intersection, 2, Scalar(0,0,255), -1);
    } else {
        int area1 = ((max_x - center_x) * (rows - max_y) - (min_y - max_y) * (cols - center_x)) / 2;
        int area2 = ((max_x - center_x) * (max_y - under_y) - (min_y - max_y) * (center_x - position_x)) / 2;
        
        intersection_x = cols + (position_x - cols) * area1 / (area1 + area2);
        intersection_y = rows + (under_y - rows) * area1 / (area1 + area2);
        Point intersection(intersection_x, intersection_y);
        
        circle(image, intersection, 2, Scalar(0,0,255), -1);
    }
}

bool isFound() {
    int size_x, size_y, p_x, p_y;
    Point intersection(intersection_x, intersection_y);
    ri->GetSize(size_x, size_y);
    ri->GetMin(p_x, p_y);
    Rect rect(p_x, p_y, p_x + size_x, p_y + size_y);
    return !(rect.contains(intersection));
}

double getAngle() {
    return angle;
}