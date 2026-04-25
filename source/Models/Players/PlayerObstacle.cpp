#include "PlayerObstacle.h"
#define PHYSICS_SCALE 50

PlayerObstacle::ItemKind PlayerObstacle::CarriedItem::kindFromType(const std::string& t) {
    return (t.rfind("spellbook", 0) == 0) ? ItemKind::SPELLBOOK : ItemKind::ARTIFACT;
}

PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s) {
    init(pos, (s * 0.4) / PHYSICS_SCALE);
    setBodyType(b2_dynamicBody);
    setDensity(1.0f);
    //setLinearDamping(15.0f);
    createFixtures();
    setAwake(true);
    setEnabled(true);
    setFixedRotation(true);
    _pid = pid;
    _speed = 4;
    _health = 6;
    _inventoryLimit = 5;
    _inventory.push_back({ "spellbook_fire", 0, CarriedItem::kindFromType("spellbook_fire"), -1, ArtifactEffect::NONE });
}

float PlayerObstacle::getSpeed() {
    return hasHeavyArtifact() ? _speed * 0.5f : _speed;
}

void PlayerObstacle::setVelocity(Vec2 v) {
    _velocity = v;
    //CULog("setVelocity called, state is %d", (int)_state);
    if (_state == AnimationState::FROZEN) {
        if (getBody()) getBody()->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }
    if (!v.isZero()) {
        _angle = v.getNormalization();

        if (_state == AnimationState::IDLE) {
            _state = AnimationState::WALKING;
        }
    }
    else if (_state == AnimationState::WALKING) {
        _state = AnimationState::IDLE;
    }

    if (!getBody()) return; // body not ready yet
    b2Vec2 currentVel = getBody()->GetLinearVelocity();
    b2Vec2 targetVel;
    if (mud){
        targetVel = b2Vec2(v.x * 0.5f, v.y * 0.5f);
    }else{
        targetVel = b2Vec2(v.x, v.y);
    }
    b2Vec2 velDiff = targetVel - currentVel;
    float mass = getBody()->GetMass();
    float scale = mass * 60.0f;
    b2Vec2 force = b2Vec2(velDiff.x * scale, velDiff.y * scale);
    //CULog("mass: %f, force: %f %f", mass, force.x, force.y);
    getBody()->ApplyForceToCenter(force, true);
}

void PlayerObstacle::setHealth(int h) {
    _health = min(h, MAX_HEALTH);

    if (_health <= 0) {
        setVelocity(Vec2::ZERO);
        _health = 0;
        _state = AnimationState::DEAD;
        setLinearVelocity(Vec2::ZERO);
    }
    else if (_state == AnimationState::DEAD && _health > 0) {
        _state = AnimationState::IDLE;
    }
}

// inventory stuff

int PlayerObstacle::getInventoryVal() const {
    int sum = 0;
    for (const auto& item : _inventory) sum += item.value;
    return sum;
}

bool PlayerObstacle::addItem(const std::string& itemType, int item_val) {
    if (!canAddItem()) return false;

    ItemKind       kind = CarriedItem::kindFromType(itemType);
    ArtifactEffect effect = (kind == ItemKind::ARTIFACT)
        ? Artifact::effectFromItemType(itemType)
        : ArtifactEffect::NONE;
    int bombTimer = (effect == ArtifactEffect::BOMB) ? 3600 : -1;

    _inventory.push_back({ itemType, item_val, kind, bombTimer, effect });
    return true;
}

int PlayerObstacle::depositItems() {
    int deposited = getInventoryVal();
    _inventory.clear();
    return deposited;
}

int PlayerObstacle::depositItem(size_t index) {
    if (index >= _inventory.size()) return 0;
    int deposited = _inventory[index].value;
    _inventory.erase(_inventory.begin() + index);
    return deposited;
}

// spell query

bool PlayerObstacle::hasSpell(SpellType spell) const {
    std::vector<std::string> required;
    switch (spell) {
    case SpellType::FIREBALL:
    case SpellType::COMBO_FIREBALL:
        return true;
    case SpellType::FREEZE:
    case SpellType::COMBO_FREEZE:
        required.push_back("spellbook_freeze");
        break;
    case SpellType::HEAL:
        required.push_back("spellbook_heal");
        break;
    case SpellType::EARTH:
        required.push_back("spellbook_earth");
        break;
    case SpellType::DEATH:
        required.push_back("spellbook_death");
        break;
    case SpellType::COMBO_EFREEZE:
        required.push_back("spellbook_earth");
        required.push_back("spellbook_freeze");
        break;
    case SpellType::WATER:
        required.push_back("spellbook_water");
        break;
    default:
        return false;
    }

    for (const auto& item : _inventory) {
        for (const auto& t : required) {
            if (item.kind == ItemKind::SPELLBOOK && item.itemType == t) {
                return true;
            }
        }
    }
    return false;
}

// artifact effects 

bool PlayerObstacle::blocksSpellcasting() const {
    for (const auto& item : _inventory) {
        if (item.kind == ItemKind::ARTIFACT &&
            item.effect == ArtifactEffect::SPELL_RESTRICTION)
            return true;
    }
    return false;
}

int PlayerObstacle::getDamageBonus() const {
    int bonus = 0;
    for (const auto& item : _inventory) {
        if (item.kind == ItemKind::ARTIFACT) {
            if (item.effect == ArtifactEffect::SPELL_BOOST)
                bonus += 2;
            else if (item.effect == ArtifactEffect::SPELL_REDUCE)
                bonus -= 2;
        }
    }
    return bonus;
}

bool PlayerObstacle::hasProtectionArtifact() const {
    for (const auto& item : _inventory) {
        if (item.kind == ItemKind::ARTIFACT &&
            item.effect == ArtifactEffect::PROTECTION)
            return true;
    }
    return false;
}

void PlayerObstacle::consumeProtectionArtifact() {
    for (auto it = _inventory.begin(); it != _inventory.end(); ++it) {
        if (it->kind == ItemKind::ARTIFACT &&
            it->effect == ArtifactEffect::PROTECTION) {
            _inventory.erase(it);
            return;
        }
    }
}

bool PlayerObstacle::hasHeavyArtifact() const {
    for (const auto& item : _inventory) {
        if (item.kind == ItemKind::ARTIFACT &&
            item.effect == ArtifactEffect::HEAVY)
            return true;
    }
    return false;
}

bool PlayerObstacle::isNonDroppable(size_t index) const {
    if (index >= _inventory.size()) return false;
    ArtifactEffect e = _inventory[index].effect;
    return e == ArtifactEffect::BOMB || e == ArtifactEffect::GOLD;
}

void PlayerObstacle::updateTimers(float dt) {
    if (_state == AnimationState::FROZEN) {
        frozenTimer -= 1;
        if (frozenTimer <= 0) {
            frozenTimer = 0;
            _state = AnimationState::IDLE;
        }
        if (getBody()) {
            getBody()->SetLinearVelocity(b2Vec2(0, 0));
        }
    }

    if (mud) {
        mudTimer -= 1;
        if (mudTimer <= 0) {
            mudTimer = 0;
            mud = false;
        }
    }

    // Bomb countdown — erase on expiry and deal damage
    for (size_t i = 0; i < _inventory.size(); ++i) {
        auto& item = _inventory[i];
        if (item.bombTimer < 0) continue;
        item.bombTimer -= 1;
        if (item.bombTimer <= 0) {
            _inventory.erase(_inventory.begin() + i);
            _bombExpired = true;
            setHealth(_health - 2);
            break;
        }
    }
}

