//
//  TutorialObstacle.h
//  Hexcapade
//
//  Created by Serena Duncan on 5/10/26.
//


#ifndef __TUT_OBS_H__
#define __TUT_OBS_H__

#include <cugl/cugl.h>

using namespace cugl;
using namespace std;

class TutorialObstacle : public physics2::BoxObstacle {
    bool _seen;
    Rect _bounds;
    std::string _png;
public:
    
    TutorialObstacle(const cugl::Rect b, std::string tex);

    static std::shared_ptr<TutorialObstacle> alloc(const cugl::Rect r, std::string tex) {
        auto f = std::make_shared<TutorialObstacle>(r, tex);
        f->createFixtures();
        return f;
    }
    
    bool getSeen() {return _seen;}
    void setSeen(bool b) {_seen = b;}
    std::string getPng() {return _png;}
    Rect getBounds() { return _bounds; }
    
};

#endif // !__TUT_OBS_H__
