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
    Rect _bounds;
    std::shared_ptr<graphics::Texture> _png;
    bool _seen;
    
public:
    TutorialObstacle(const cugl::Rect b, std::shared_ptr<graphics::Texture> tex);

    static std::shared_ptr<TutorialObstacle> alloc(const cugl::Rect r, std::shared_ptr<graphics::Texture> tex) {
        auto f = std::make_shared<TutorialObstacle>(r, tex);
        f->createFixtures();
        return f;
    }
    
    bool getSeen() {return _seen;}
    void setSeen(bool b) {_seen = b;}
    std::shared_ptr<graphics::Texture> getPng() {return _png;}
    
};

#endif // !__TUT_OBS_H__
