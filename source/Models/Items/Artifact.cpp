//
//  Artifact.cpp
//  alpha_final
//
//  Created by Frank Xie on 4/24/26
//
#include "Artifact.h"

ArtifactEffect Artifact::effectFromItemType(const std::string& itemType) {
    if (itemType == "artifact_scroll")
        return ArtifactEffect::SPELL_RESTRICTION;
    if (itemType == "artifact_gem_boost")
        return ArtifactEffect::SPELL_BOOST;
    if (itemType == "artifact_gem_reduce")
        return ArtifactEffect::SPELL_REDUCE;
    if (itemType == "artifact_protection")
        return ArtifactEffect::PROTECTION;
    if (itemType == "artifact_heavy")
        return ArtifactEffect::HEAVY;
    if (itemType == "artifact_gold_real")
        return ArtifactEffect::GOLD;
    if (itemType == "artifact_gold_fake")
        return ArtifactEffect::GOLD;
    if (itemType == "artifact_bomb")
        return ArtifactEffect::BOMB;
    return ArtifactEffect::NONE;
}

std::string Artifact::textureKeyFromEffect(ArtifactEffect effect) {
    switch (effect) {
    case ArtifactEffect::SPELL_RESTRICTION:
        return "artifact_scroll";
    case ArtifactEffect::SPELL_BOOST:
        return "artifact_gem_boost";
    case ArtifactEffect::SPELL_REDUCE:
        return "artifact_gem_reduce";
    case ArtifactEffect::PROTECTION:
        return "artifact_protection";
    case ArtifactEffect::HEAVY:
        return "artifact_heavy";
    case ArtifactEffect::GOLD:
        return "artifact_gold";
    case ArtifactEffect::BOMB:
        return "artifact_bomb";
    case ArtifactEffect::NONE:
    default:
        return "";
    }
}