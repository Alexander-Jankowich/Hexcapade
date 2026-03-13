//
//  Artifact.cpp
//  Hexcapade
//
//  Prototype artifact implementation.
//
#include "Artifact.h"

using namespace cugl;

Artifact::Artifact(std::string id, std::string name, int value, Effect effect) :
Item(std::move(id), std::move(name), value, Item::Kind::ARTIFACT),
_effect(effect) {
}

bool Artifact::blocksSpellcasting() const {
    // // GA: itemnet
    // Artifact effect gameplay is intentionally stubbed until the team ports the
    // spell/artifact interaction rules onto the networking branch.
    return false;
}

float Artifact::getDamageMultiplierModifier() const {
    // // GA: itemnet
    // Returning zero keeps the current combat balance unchanged while we focus
    // this pass on synchronized pickup/drop/deposit state.
    return 0.0f;
}

bool Artifact::hasProtectionTrigger() const {
    // // GA: itemnet
    // Protection behavior is deferred until the artifact gameplay rules land.
    return false;
}

Color4 Artifact::getDebugColor() const {
    return Color4(255, 170, 60, 255);
}

const char* Artifact::effectToString(Effect effect) {
    (void)effect;
    return "artifact_effect";
}
