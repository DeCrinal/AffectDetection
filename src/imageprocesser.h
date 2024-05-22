#ifndef IMAGEPROCESSER_H
#define IMAGEPROCESSER_H
#include <opencv2/opencv.hpp>
#include <iostream>
#include <optional>
#include <map>

class AreasContainer;

///< Перечисление возможных стадий обработки изображения
enum ImageStage : uint8_t{
    Original,
    Gray,
    RemovedShadow,
    Filled,
    Blured,
    BinImage,
    RGB,
    FinalImage
};

class ImageProcesser
{
    ///< Ожидаемый размер заполняемой части
    int mRectSize;
    ///< Ожидаемая заполняемая часть
    double mFillingPart;
    ///< Ожидаемый коэффициент заполнения области
    int mFillRate;

    std::map<ImageStage, cv::Mat> mAllImagesInStages;

    AreasContainer *mAreaContainer {nullptr};
public:
    /// Конструктор объекта
    ImageProcesser();
    ///< Метод считывает изображение из директории по умолчанию
    int readImageFromDir();
    ///< Возвращает количество черных пикселов в указанном изображении image
    int countBlackPixels(const cv::Mat& image);
    ///< Применяет размытие в квадрате размером kSize, times раз
    void applyMedianBlur(int times, int kSize);
    ///< Применяет бинаризацию
    void applyThreshold(int low, int high);
    ///< Метод возвращает изображение
    cv::Mat getImage(ImageStage stage) const;
    ///< Показывает изображение указанной стадии
    void showImage(ImageStage stage) const;
    ///< Метод заполняет пустоты внутри областей дефектов
    void fillEmptinesInAreas();
    ///< Метод осуществляет инициализацию обсчитывающего контенера точками из входного изображения
    void initAreaContainer();
    ///< Метод осуществляет генерацию финального изображения.
    /// На оригинальное изображение добавляются:
    /// 1. Границы областей дефектов
    /// 2. Обрамляющие их прямоугольники
    /// 3. Подписи дефектов
    void generateFinalImage();
    ///< Возвращает целочисленное значение количества пикселей в квадрате размером size*part
    /// part - в диапозоне [0:1]
    int getPercentOfSquare(int size, double part);
    void removeShadow(int borderSize);
private:
    ///< Метод осуществляет заполнение области в указанных координатах (x0, y0) квадратом
    /// размера size в изображении image
    void fillRectByCoord(cv::Mat& image, int y0, int x0, int size);
    ///< Метод показывает указанное изображение в окне с заголовком titile и сохраняет его
    /// по полному имени filename
    void showAndSave(const cv::Mat image, const std::string &title, const std::string &filename) const;
};

#endif // IMAGEPROCESSER_H
