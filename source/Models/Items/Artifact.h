//
//  Artifact.h
//  alpha_final
//
//  Created by Frank Xie on 4/24/26
//
#ifndef __ARTIFACT_H__
#define __ARTIFACT_H__

#include <string>

enum class ArtifactEffect {
    /** No effect. */
    NONE,
    /** Player cannot cast spells. */
    SPELL_RESTRICTION,
    /** Attack spells deal +2 damage. */
    SPELL_BOOST,
    /** Attack spells deal -2 damage. */
    SPELL_REDUCE,
    /** Tanks one lethal hit, then consumed. */
    PROTECTION,
    /** Player moves at half speed. */
    HEAVY,
    /** High-value item (real). Non-droppable. */
    GOLD,
    /** Explodes after a timer, dealing damage. Non-droppable. */
    BOMB,
};

class Artifact {
public:
    /**
     * Maps an itemType string to its ArtifactEffect. R
     * This is the single source of truth — on place we hardcode these strings.
     */
    static ArtifactEffect effectFromItemType(const std::string& itemType);

    /**
     * Maps an ArtifactEffect back to the asset key used to load its texture.
     * Proc gen can use this when it needs to spawn an artifact with a specific effect without hardcoding strings.
     */
    static std::string textureKeyFromEffect(ArtifactEffect effect);
};

#endif // __ARTIFACT_H__
