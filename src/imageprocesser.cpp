#include "imageprocesser.h"
#include "areascontainer.h"
#include "imageprocesser.h"

void ImageProcesser::fillRectByCoord(cv::Mat &image, int y0, int x0, int size){
    const int yN = y0 + size;
    const int xN = x0 + size;

    for(int y = y0; y < yN; y++){
        for(int x = x0; x < xN; x++){
            image.at<uchar>(y, x) = 0;
        }
    }
}

ImageProcesser::ImageProcesser()
{
    mRectSize = 4;
    mFillingPart = 0.1;
    mFillRate = getPercentOfSquare(mRectSize, mFillingPart);
    mAreaContainer = new AreasContainer;
}

int ImageProcesser::countBlackPixels(const cv::Mat &image) {
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

void ImageProcesser::applyMedianBlur(int times, int kSize){
    cv::Mat result = mAllImagesInStages.at(Filled);
    for(int i = 0; i < times; i++){
        cv::medianBlur(result,  result, kSize);
    }
    mAllImagesInStages[Blured] = result;
}

void ImageProcesser::applyThreshold(int low, int high)
{
    cv::threshold(mAllImagesInStages[Blured], mAllImagesInStages[BinImage], low, high, cv::THRESH_BINARY);
}

int ImageProcesser::getPercentOfSquare(int size, double part){
    return size * size * part;
}

void ImageProcesser::removeShadow(int borderSize)
{
    cv::Mat result = mAllImagesInStages.at(Gray);
    for(int y = 0; y < result.rows; y++){
        for(int x = 0; x < result.cols; x++){
            if(y < borderSize || x < borderSize || abs(x - result.cols) < borderSize ||
               abs(y - result.rows) < borderSize){
                result.at<uchar>(y, x) = 255;
            }
        }
    }
    mAllImagesInStages[RemovedShadow] = result;
}

void ImageProcesser::showImage(ImageStage stage) const
{
    if(mAllImagesInStages.find(stage) == mAllImagesInStages.end()){
        return;
    }
    std::string title;
    switch(stage){
    case ImageStage::Original :
        title = "Original Image";
        break;
    case ImageStage::FinalImage :
        title = "Final Image";
        break;
    case ImageStage::BinImage :
        title = "Binary Image";
        break;
    case ImageStage::RemovedShadow :
        title = "Removed shadow";
        break;
    case ImageStage::Blured :
        title = "Blured Image";
        break;
    case ImageStage::Filled :
        title = "Filled Image";
        break;
    case ImageStage::Gray :
        title = "Gray Image";
        break;
    case ImageStage::RGB :
        title = "Colored Image";
        break;
    default:
        break;
    }

    showAndSave(mAllImagesInStages.at(stage), title, "./step_" + std::to_string(static_cast<int>(stage)) + ".jpg");
}

cv::Mat ImageProcesser::getImage(ImageStage stage) const
{
    return mAllImagesInStages.at(stage);
}

void ImageProcesser::fillEmptinesInAreas(){
    //cv::Mat imageProc = mAllImagesInStages[Gray];
    cv::Mat imageProc = mAllImagesInStages[RemovedShadow];
    for(int y = 1; y < imageProc.rows - mRectSize; y += mRectSize){
        for(int x = 1; x < imageProc.cols - mRectSize; x += mRectSize){
            cv::Rect windowRect(x, y, mRectSize, mRectSize);
            cv::Mat window = imageProc(windowRect);
            int blackPixelCount = countBlackPixels(window);

            if(blackPixelCount < mFillRate){
                continue;
            }

            fillRectByCoord(imageProc, y, x, mRectSize);
        }
    }
    mAllImagesInStages[Filled] = imageProc;
}

void ImageProcesser::initAreaContainer(){
    const cv::Mat image = mAllImagesInStages.at(BinImage);
    mAreaContainer->beginUpdateContainer();
    for (int y = 0; y < image.rows; y++){
        for (int x = 0; x < image.cols; x++){
            if (image.at<uchar>(y, x) == 0){
                mAreaContainer->addPoint({x, y});
            }
        }
    }
    mAreaContainer->endUpdateContainer();
}

void ImageProcesser::generateFinalImage(){
    const set<pair<int, int>> borders = mAreaContainer->getBorderPoints();
    const set<shared_ptr<Area>> areas = mAreaContainer->getAreas();
    cv::Mat imageGRB = mAllImagesInStages[RGB];
    size_t counter{0};

    // Цикл обработки каждой области дефекта и добавления ее параметров на изображение
    for(const auto &area : areas){
        const std::vector<std::vector<cv::Point>> contours = area->getBoardingRectContours();
        const cv::RotatedRect rect = area->getBoardingRect();

        // Отрисовка ограничивающего прямоугольника
        cv::drawContours(imageGRB, contours, 0, cv::Scalar(0, 255, 0), 2);


        // Генерация названия области дефекта
        vector<std::string> strsToPut;
        strsToPut.push_back("Fig." + std::to_string(++counter));
        Area::AreaType type = area->getAreaType();
        if(type == Area::AreaType::Point){
            strsToPut.push_back("Type: Point "
                                + std::to_string(area->getMaximalDimensial()) + "mm");
        }
        else if(type == Area::AreaType::Scratch){
            strsToPut.push_back("Scratch "
                                + std::to_string(area->getMinimalDimensial()) + "mm");
        }
        else if(type == Area::AreaType::Zone){
            strsToPut.push_back(
                        "Notfilling ("
                        + std::to_string(100 * (float)area->getSquare() / (imageGRB.cols * imageGRB.rows)).substr(0,4)
                        + "%)");
        }

        // Добавление номера области дефекта
        cv::putText(imageGRB, strsToPut[0], cv::Point(rect.center.x, rect.center.y - rect.size.width/2),
                cv::FONT_ITALIC, 0.5, cv::Scalar(127, 0, 127), 2, cv::LINE_AA);
        // Добавление названия областия дефекта
        cv::putText(imageGRB, strsToPut[1], cv::Point(rect.center.x, rect.center.y - rect.size.width/2 + 20),
                cv::FONT_ITALIC, 0.5, cv::Scalar(127, 0, 127), 2, cv::LINE_AA);
    }

    // Добавление границ области дефекта
    for(const auto &boardPt : borders){
        cv::circle(imageGRB, cv::Point(boardPt.first, boardPt.second), 1, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
    }

    mAllImagesInStages[FinalImage] = imageGRB;
}

void ImageProcesser::showAndSave(const cv::Mat image, const std::string &title, const std::string &filename) const
{
    cv::imshow(title, image);
    cv::waitKey(0);
    cv::imwrite(filename, image);
}

int ImageProcesser::readImageFromDir(){
    const cv::Mat imageOriginal = cv::imread("/home/rai/Documents/VirtualAssist/AffectDetection/pics/clearPic2.jpg",
                                             cv::IMREAD_GRAYSCALE);

    if(imageOriginal.empty()){
        return 1;
    }

    mAllImagesInStages[Original] = imageOriginal;
    cv::medianBlur(mAllImagesInStages[Original], mAllImagesInStages[Gray], 5);
    cv::Mat blurredMat;

    cv::GaussianBlur(mAllImagesInStages[Gray], blurredMat, cv::Size(5, 5), 0);
    mAllImagesInStages[Gray] = blurredMat;


    cv::Mat adaptiveThresholded;
    cv::adaptiveThreshold(mAllImagesInStages[Gray], adaptiveThresholded, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 51, 2);

    mAllImagesInStages[Gray] = adaptiveThresholded;

    cv::Mat medianBlurred1, medianBlurred2;
    cv::medianBlur(mAllImagesInStages[Gray], medianBlurred1, 15);
    cv::medianBlur(medianBlurred1, medianBlurred2, 9);
    mAllImagesInStages[Gray] = medianBlurred2;

    mAllImagesInStages[RGB] = cv::Mat(imageOriginal.size(), CV_8UC3);
    cv::cvtColor(mAllImagesInStages[Original], mAllImagesInStages[RGB], cv::COLOR_GRAY2RGB);

    return 0;
}
