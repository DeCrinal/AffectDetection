#include "areascontainer.h"

int Area::mID = 0;

bool Area::isOnBorder(const pair<int, int> &point) const{
    bool isBorder {false};
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int i = 0; i < 8; ++i) {
        int newX = point.first + dx[i];
        int newY = point.second + dy[i];

        isBorder = isBorder ||
                (mPoints.find({newX, newY}) != mPoints.end()); //Если нашел
    }

    return isBorder;
}

void Area::printPoints(bool needPrintID) const
{
    if(needPrintID){
        cout << mID << endl;
    }
    for(const auto &coord : mPoints){
        cout << "(" << coord.first << ", " << coord.second << ")" << endl;
    }
}

vector<vector<cv::Point> > Area::getBoardingRectContours() const
{
    return mBoardingRectContours;
}

RotatedRect Area::getBoardingRect() const
{
    return mBoardingRect;
}

Area::Area()
{
    mID++;
}

Area::~Area()
{
    mID--;
}

void Area::append(const pair<int, int> &point){
    mPoints.insert(point);
}

int Area::getSquare()const{
    //return m_square;
    return mPoints.size();
}

bool Area::isBorder(const pair<int, int> &point) const
{
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int i = 0; i < 8; ++i) {
        int newX = point.first + dx[i];
        int newY = point.second + dy[i];

        if(mPoints.find({newX, newY}) == mPoints.end()){
            return true;
        }
    }

    return false;
}

set<pair<int, int> > Area::getBorderPoints() const
{
    set<pair<int, int>> borderPoints;
    for(const auto &pt : mPoints){
        if(isBorder(pt)){
            borderPoints.insert(pt);
        }
    }

    return borderPoints;
}

set<pair<int, int>> Area::getPoints() const{
    return  mPoints;
}

void Area::merge(const Area &ar){
    merge(ar.getPoints());
}

pair<int, int> Area::getBaricenter() const
{
    return mBariCenter;
}

Area::AreaType Area::getAreaType() const
{
    return mAreaType;
}

void Area::updateCharacticParams()
{
    long long xSum {0};
    long long ySum {0};
    for(const auto &pt : mPoints){
        xSum += pt.first;
        ySum += pt.second;
    }
    mBariCenter = {xSum/mPoints.size(), ySum/mPoints.size()};


    vector<Point2f> rectCords;
    for(const auto &pt: mPoints){
        rectCords.push_back({static_cast<float>(pt.first), static_cast<float>(pt.second)});
    }

    mBoardingRect = cv::minAreaRect(rectCords);
    cv::Point2f box[4];
    mBoardingRect.points(box);
    vector<cv::Point> boxPoints(std::begin(box), std::end(box));
    mBoardingRectContours = {boxPoints};

    if(mBoardingRect.size.width < POINT_MAX_MEASURE && mBoardingRect.size.height < POINT_MAX_MEASURE){
        mAreaType = AreaType::Point;
    }
    else if(    (mBoardingRect.size.height < SCRATCH_MAX_WIDTH && mBoardingRect.size.width > SCRATCH_MIN_LENGTH)
            || 	(mBoardingRect.size.width < SCRATCH_MAX_WIDTH && mBoardingRect.size.height > SCRATCH_MIN_LENGTH) ){
        mAreaType = AreaType::Scratch;
    }
    else{
        mAreaType = AreaType::Zone;
    }
}

void Area::merge(const std::set<pair<int, int>> &points)
{
    for(const auto &pt : points){
        mPoints.insert(pt);
    }
}

void Area::merge(const std::shared_ptr<Area> &areaPtr)
{
    merge(areaPtr->getPoints());
}

AreasContainer::AreasContainer()
{

}

AreasContainer::~AreasContainer()
{

}

void AreasContainer::addPoint(const std::pair<int, int> &point)
{
    vector<shared_ptr<Area>> boardingAreas;
    for(const auto &area : mAreas ){
        if(area->isOnBorder(point)){
            if(std::find(boardingAreas.begin(), boardingAreas.end(), area) ==
                    boardingAreas.end()){
                boardingAreas.push_back(area);
            }
        }
    }

    if(boardingAreas.empty()){
        const shared_ptr<Area> newArea = std::make_shared<Area>();
        newArea->append(point);
        mAreas.insert(newArea);
        return;
    }

    for(size_t i = 1; i < boardingAreas.size(); i++){
        boardingAreas[0]->merge(boardingAreas[i]);
    }

    for(size_t i = 1; i < boardingAreas.size(); i++){
        mAreas.erase(boardingAreas[i]);
    }

    boardingAreas[0]->append(point);
    mAreas.insert(boardingAreas[0]);
}

void AreasContainer::printAllAreas() const
{
    for(const auto &area : mAreas){
        area->printPoints(true);
    }
}

void AreasContainer::printAllAreasSizes() const
{
    for(const auto &area : mAreas){
        cout << area->getSquare() << endl;
    }
}

int AreasContainer::getAreasNumber() const
{
    return mAreas.size();
}

set<pair<int, int> > AreasContainer::getBorderPoints() const
{
    set<pair<int, int>> borderPoints;

    for(const auto &area : mAreas){
        const auto &pts = area->getBorderPoints();
        for(const auto &pt : pts){
            borderPoints.insert(pt);
        }
    }

    return borderPoints;
}

vector<pair<int, int> > AreasContainer::getAreasBaricenters() const
{
    vector<pair<int, int>> baricenters;
    for(const auto &area : mAreas){
        baricenters.push_back(area->getBaricenter());
    }
    return baricenters;
}

set<shared_ptr<Area> > AreasContainer::getAreas() const
{
    return mAreas;
}

void AreasContainer::beginUpdateContainer()
{
    mStatus = Updatintg;
}

void AreasContainer::endUpdateContainer()
{
    for(const auto &area : mAreas){
        area->updateCharacticParams();
    }
    mStatus = Done;
}
