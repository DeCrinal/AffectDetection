#include <iostream>
#include <opencv2/opencv.hpp>
#include "areascontainer.h"

using namespace std;
using namespace cv;

cv::Mat applyMedianBlur(const cv::Mat& image, int times, int kSize){
    cv::Mat result;
    result = image;
    for(int i = 0; i < times; i++){
        cv::medianBlur(result,  result, kSize);
    }
    return result;
}

int getPercentOfSquare(int size, double part){
    return size * size * part;
}

int customCountNonZero(const Mat& image) {
    int count = 0;
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            if (image.at<uchar>(y, x) == 0) {
                count++;
            }
        }
    }
    return count;
}

void fillRectByCoord(cv::Mat& image, int y0, int x0, int size){
    const int yN = y0 + size;
    const int xN = x0 + size;

    for(int y = y0; y < yN; y++){
        for(int x = x0; x < xN; x++){
            image.at<uchar>(y, x) = 0;
        }
    }
}

int main()
{
    const cv::Mat imageOriginal = cv::imread("/home/rai/Documents/VirtualAssist/Part3/Part3/Part3_04_2.jpg",
                               IMREAD_GRAYSCALE);

    if(imageOriginal.empty()){
        std::cerr << "Error: Couldn't open the image file" << std::endl;
        return 1;
    }

    const int rectSize = 4;
    const int fillRate = getPercentOfSquare(rectSize, 0.1);

    cv::imshow("OriginalImage", imageOriginal);
    cv::waitKey(0);
    cv::imwrite("./step0.jpg", imageOriginal);
    cv::Mat imageGray = imageOriginal;
    for(int y = 1; y < imageGray.rows - rectSize; y += rectSize){
        for(int x = 1; x < imageGray.cols - rectSize; x += rectSize){
            cv::Rect windowRect(x, y, rectSize, rectSize);
            cv::Mat window = imageGray(windowRect);
            int blackPixelCount = customCountNonZero(window);

            if(blackPixelCount < fillRate){
                continue;
            }

            fillRectByCoord(imageGray, y, x, rectSize);
        }
    }

    cv::imshow("Image", imageGray);
    cv::imwrite("./step1.jpg", imageGray);
    cv::waitKey(0);

    imageGray = applyMedianBlur(imageGray, 1, 5);
    cv::Mat binImage;
    cv::threshold(imageGray, binImage, 128, 255, cv::THRESH_BINARY);

    cv::imshow("Binary Image", binImage);
    cv::imwrite("./step2.jpg", imageGray);
    cv::waitKey(0);

    Mat labeledImage;
    Mat stats, centroids;


    AreasContainer arCont;

    arCont.beginUpdateContainer();
    for (int y = 0; y < binImage.rows; y++){
        for (int x = 0; x < binImage.cols; x++){
            if (binImage.at<uchar>(y, x) == 0){
                arCont.addPoint({x, y});
            }
        }
    }
    arCont.endUpdateContainer();


    arCont.printAllAreasSizes();

    const set<pair<int, int>> borders = arCont.getBorderPoints();
    vector<cv::Point> borders2;
    for(const auto &pt : borders){
        borders2.push_back({pt.first, pt.second});
    }

    const vector<pair<int, int>> baricenters = arCont.getAreasBaricenters();
    vector<cv::Point> cvBaricents;
    for(const auto &pt : baricenters){
        cvBaricents.push_back({pt.first, pt.second});
    }


    cv::Scalar color(0, 0, 255);
    cv::Mat img_grb(imageOriginal.size(), CV_8UC3);
    cv::cvtColor(imageOriginal, img_grb, cv::COLOR_GRAY2RGB);

    const set<shared_ptr<Area>> areas = arCont.getAreas();
    size_t counter{0};
    for(const auto &area : areas){
        std::vector<std::vector<cv::Point>> contours = area->getBoardingRectContours();
        cv::RotatedRect rect = area->getBoardingRect();
        cv::drawContours(img_grb, contours, 0, cv::Scalar(0, 255, 0), 2);
        vector<std::string> strsToPut;
        strsToPut.push_back("Fig." + std::to_string(++counter));
        Area::AreaType type = area->getAreaType();
        if(type == Area::AreaType::Point){
            strsToPut.push_back("Type: Point");
        }
        else if(type == Area::AreaType::Scratch){
            strsToPut.push_back("Scratch");
        }
        else if(type == Area::AreaType::Zone){
            strsToPut.push_back(
                      "Notfilling ("
                    + std::to_string(100 * (float)area->getSquare() / (img_grb.cols * img_grb.rows)).substr(0,4)
                    + "%)");
        }
        cv::putText(img_grb, strsToPut[0], cv::Point(rect.center.x, rect.center.y - rect.size.width/2),
                    cv::FONT_ITALIC, 0.5, cv::Scalar(127, 0, 127), 2, LINE_AA);
        cv::putText(img_grb, strsToPut[1], cv::Point(rect.center.x, rect.center.y - rect.size.width/2 + 20),
                    cv::FONT_ITALIC, 0.5, cv::Scalar(127, 0, 127), 2, LINE_AA);
    }


    for(const auto &boardPt : borders){
        // Последний параметр -1 означает, что круг заполняется
        cv::circle(img_grb, cv::Point(boardPt.first, boardPt.second), 1, cv::Scalar(0, 0, 255), -1, LINE_AA);
    }
    /*for(const auto &barC : baricenters){
        cv::circle(img_grb, cv::Point(barC.first, barC.second), 5, cv::Scalar(255, 0, 255), -1, LINE_AA);
    }*/

    cv::imshow("Bordered image", img_grb);
    cv::imwrite("./step3.jpg", img_grb);
    cv::waitKey(0);

    return 0;
}
