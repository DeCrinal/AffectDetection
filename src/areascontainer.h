#ifndef AREASCONTAINER_H
#define AREASCONTAINER_H

#include <iostream>
#include <algorithm>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

using std::pair;
using std::set;
using std::list;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;
using cv::RotatedRect;
using cv::Point2f;

class Area{

public:
    ///! Классификационный тип области
    enum class AreaType{
        Undefined = 0, ///< Неопределено
        Zone = 1, ///< Незаполненность
        Scratch = 2, ///< Трещина
        Point = 3 ///< Выбоина
    };

private:
    ///! Минимальная длина трещены
    static const int SCRATCH_MIN_LENGTH {50};
    ///! Максимальная ширина трещены
    static const int SCRATCH_MAX_WIDTH {20};
    ///! Максимальная мера выбоины
    static const int POINT_MAX_MEASURE {15};
    ///! Уникальный-идентификатор области
    static int mID;

    ///! Тип текущей области
    AreaType mAreaType {AreaType::Undefined};
    ///! Множество точек заданной области
    set<pair<int, int>> mPoints;
    ///! Барицентер заданной области
    pair<int, int> mBariCenter {0, 0};
    ///! Минимальный прямоугольник, внутри которого находится область
    RotatedRect mBoardingRect;
    vector<vector<cv::Point>> mBoardingRectContours;
    ///! Площадь текущей области
    int mSquare {0};


public:
    Area();
    ~Area();

    ///! Добавляет точку в область
    void append(const pair<int, int> &point);
    ///! Возвращает площадь
    int getSquare()const;
    ///! Возвращает множество точек-границ текущей области
    set<pair<int, int>> getBorderPoints() const;
    ///! Возвращает множество точек текущей области
    set<pair<int, int>> getPoints() const;
    ///! Метод определяет - находится ли входная точка на границе уже
    /// существующих
    bool isOnBorder(const pair<int, int> &point) const;
    ///! Метод определяет - является ли точка граничной
    bool isBorder(const pair<int, int> &point) const;
    ///! Печатает все точки текущей области в стандартный вывод
    void printPoints(bool needPrintID = false) const;

    ///! Возвращает границы обрамляющего прямоугольника
    vector<vector<cv::Point>> getBoardingRectContours() const;
    RotatedRect getBoardingRect() const;
    ///! Метод слияния
    void merge(const shared_ptr<Area> &areaPtr);
    ///! Метод слияния
    void merge(const set<pair<int, int>> &points);
    ///! Метод слияния
    void merge(const Area& ar);
    ///! Метод получения барицентров
    pair<int, int> getBaricenter() const;
    ///! Метод получения типа области
    AreaType getAreaType() const;

private:
    friend class AreasContainer;
    void updateCharacticParams();
};


class AreasContainer{
    set<shared_ptr<Area>> mAreas;

    ///! Вспомогательный статус для оптимизации времени обновления данных
    enum uint8_t{
        Updatintg,
        Done
    } mStatus {Done};

public:
    AreasContainer();
    ~AreasContainer();

    ///! Распределяет заданную точку в подходящую область. При необходимости
    /// создается новая область.
    ///
    /// Прим.: при использовании для обновления внутренних состояний требуется
    /// вызвать пару методов beginUpdateContainer() и endUPdateContainer()
    void addPoint(const pair<int, int> &point);
    ///! Выводит все области в стандартный вывод
    void printAllAreas() const;
    ///! Выводит площади всех областей
    void printAllAreasSizes() const;
    ///! Возвращает количество областей
    int getAreasNumber() const;
    ///! Возвращает множество всех граничных точек
    set<pair<int, int>> getBorderPoints() const;
    ///! Возвращает барицентры всех областей
    vector<pair<int, int>> getAreasBaricenters() const;
    ///! Возвращает все области
    set<shared_ptr<Area>> getAreas() const;
    vector<vector<cv::Point>> getBoardingRectsContours() const;
    ///! Вспомогательный метод. Используется для оптимизации обновления
    /// внутренних состояний
    void beginUpdateContainer();
    ///! Вспомогательный метод. Используется для оптимизации обновления
    /// внутренних состояний
    void endUpdateContainer();

private :
};
#endif // AREASCONTAINER_H
