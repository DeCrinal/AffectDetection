#include <iostream>
#include "imageprocesser.h"

int main()
{
    ImageProcesser imageProcesser;
    const auto readRes = imageProcesser.readImageFromDir();
    if(readRes){
        return 1;
        std::cout << "Ошибка чтения файла!";
    }

    imageProcesser.showImage(ImageStage::Original);
    imageProcesser.showImage(ImageStage::Gray);

    imageProcesser.removeShadow(15);
    imageProcesser.showImage(ImageStage::RemovedShadow);

    //Заполнение пустот внутри областей
    imageProcesser.fillEmptinesInAreas();
    //Вывод изображения
    imageProcesser.showImage(ImageStage::Filled);

    //Применение размытия
    //imageProcesser.applyMedianBlur(1, 5);
    imageProcesser.applyMedianBlur(1, 15);
    //Вывод изображения
    imageProcesser.showImage(ImageStage::Blured);

    //Применение бинаризации
    imageProcesser.applyThreshold(128, 255);
    //Вывод изображение
    imageProcesser.showImage(ImageStage::BinImage);

    //Заполнение контейнеров для обсчета и идентификации областей дефектов и их
    //типов
    imageProcesser.initAreaContainer();

    //Генерация финального изображение
    imageProcesser.generateFinalImage();
    //Вывод финального изображения
    imageProcesser.showImage(ImageStage::FinalImage);


    return 0;
}
