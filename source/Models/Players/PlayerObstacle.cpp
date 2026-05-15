#include "PlayerObstacle.h"
#define PHYSICS_SCALE 50.0f
#define HURT_TIME 84


PlayerObstacle::PlayerObstacle(Vec2 pos, string pid, cugl::Size s, const shared_ptr<JsonValue>& consts) {
    init(pos, (s * 0.12) / PHYSICS_SCALE);
    setBodyType(b2_dynamicBody);
    setDensity(1.0f);
    //setLinearDamping(15.0f);

    createFixtures();
    setAwake(true);
    setEnabled(true);
    setFixedRotation(true);

    _curSpell = SpellType::NONE;

    _pid = pid;
    _speed = 4;
    _health = 6;
    _inventoryLimit = 5;
    _inventory.push_back(CarriedItem(ItemTypeClass::ItemType::BOOK_EARTH, consts->getInt("value"), -1));
}

void PlayerObstacle::setVelocity(Vec2 v) {
    _velocity = v;
    //CULog("setVelocity called, state is %d", (int)_state);
    if (isFrozen()) {
        if (getBody()) getBody()->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }

    if (!v.isZero()) {
        _angle = v.getNormalization();
    }

    if (!getBody()) return; // body not ready yet
    b2Vec2 currentVel = getBody()->GetLinearVelocity();
    b2Vec2 targetVel;

    if (_mud){
        targetVel = b2Vec2(v.x * 0.5f, v.y * 0.5f);
    }
    else{
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
    _dirty = true;
    if (h < _health) {
        _hurt = true;
    }

    _health = min(h, MAX_HEALTH);

    if (_health <= 0) {
        setVelocity(Vec2::ZERO);
        setLinearVelocity(Vec2::ZERO);
        
        _health = 0;
    }
}

void PlayerObstacle::damage(float damage) {
    if (_iframeTimer > 0) {
        // Cannot be hit if already hit very recently
        CULog("have iframes, %d", _iframeTimer);
        return;
    }
    _iframeTimer = HURT_TIME;

    if (_shield) {
        setShieldCount(getShieldCount() - 1);
        if (getShieldCount() == 0) {
            setShield(false);
        }
        return;
    }
    if (_health - damage <= 0 && hasProtectionArtifact()) {
        consumeItem(ItemTypeClass::ItemType::PROTECTION);
        setHealth(2);
        return;
    }
    setHealth(_health - damage);
}

void PlayerObstacle::updateIframeTimer() {
    if (_iframeTimer > 0) {
        _iframeTimer--;
    }
}

// inventory stuff

int PlayerObstacle::getInventoryVal() const {
    int sum = 0;
    for (const auto& item : _inventory) sum += item.value;
    return sum;
}

bool PlayerObstacle::addItem(ItemTypeClass::ItemType itemType, int item_val) {
    if (!canAddItem()) return false;

   
    int bombTimer = (itemType == ItemTypeClass::ItemType::BOMB) ? 1200 : -1;

    _inventory.push_back(CarriedItem(itemType, item_val, bombTimer));
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
    std::vector<ItemTypeClass::ItemType> required;
    switch (spell) {
    case SpellType::FIREBALL:
    case SpellType::COMBO_FIREBALL:
        return true;
    case SpellType::FREEZE:
    case SpellType::COMBO_FREEZE:
        required.push_back(ItemTypeClass::ItemType::BOOK_FREEZE);
        break;
    case SpellType::HEAL:
        required.push_back(ItemTypeClass::ItemType::BOOK_HEAL);
        break;
    case SpellType::EARTH:
        required.push_back(ItemTypeClass::ItemType::BOOK_EARTH);
        break;
    case SpellType::DEATH:
        required.push_back(ItemTypeClass::ItemType::BOOK_DEATH);
        break;
    case SpellType::COMBO_EFREEZE:
        required.push_back(ItemTypeClass::ItemType::BOOK_EARTH);
            required.push_back(ItemTypeClass::ItemType::BOOK_FREEZE);
        break;
    case SpellType::WATER:
        required.push_back(ItemTypeClass::ItemType::BOOK_FREEZE);
        break;
    default:
        return false;
    }

    for (const auto& item : _inventory) {
        for (const auto& t : required) {
            if (item.itemType == t) {
                return true;
            }
        }
    }
    return false;
}

// artifact effects 

bool PlayerObstacle::blocksSpellcasting() const {
    for (const auto& item : _inventory) {
        if (item.itemType == ItemTypeClass::ItemType::SPELL_RESTRICTION)
            return true;
    }
    return false;
}

int PlayerObstacle::getDamageBonus() const {
    int bonus = 0;
    for (const auto& item : _inventory) {
        if (item.itemType == ItemTypeClass::ItemType::SPELL_BOOST)
            bonus += 2;
        else if (item.itemType == ItemTypeClass::ItemType::SPELL_REDUCE)
            bonus -= 2;
    }
    return bonus;
}

bool PlayerObstacle::hasProtectionArtifact() const {
    for (const auto& item : _inventory) {
        if (item.itemType == ItemTypeClass::ItemType::PROTECTION)
            return true;
    }
    return false;
}

void PlayerObstacle::consumeItem(ItemTypeClass::ItemType itemType) {
    for (auto it = _inventory.begin(); it != _inventory.end(); ++it) {
        if (it->itemType == itemType) {
            _inventory.erase(it);
            return;
        }
    }
}

bool PlayerObstacle::hasHeavyArtifact() const {
    for (const auto& item : _inventory) {
        if (item.itemType == ItemTypeClass::ItemType::HEAVY)
            return true;
    }
    return false;
}

bool PlayerObstacle::isNonDroppable(size_t index) const {
    if (index >= _inventory.size()) return false;
    ItemTypeClass::ItemType e = _inventory[index].itemType;
    return e == ItemTypeClass::ItemType::BOMB || e == ItemTypeClass::ItemType::GOLD;
}

void PlayerObstacle::updateTimers(float dt) {
    if (_frozen) {
        _frozenTimer -= 1;
        if (_frozenTimer <= 0) {
            _frozenTimer = 0;
            _frozen = false;
            _dirty = true;
        }
        if (getBody()) {
            getBody()->SetLinearVelocity(b2Vec2(0, 0));
        }
    }

    if (_mud) {
        _mudTimer -= 1;
        if (_mudTimer <= 0) {
            _mudTimer = 0;
            _mud = false;
            _dirty = true;
        }
    }
    if(_water){
        _waterTimer -= 1;
        if (_waterTimer <= 0) {
            _waterTimer = 0;
            _water = false;
            _waterPushDir = Vec2::ZERO;
        }
    }

    // Bomb countdown — erase on expiry and deal damage
    for (size_t i = 0; i < _inventory.size(); ++i) {
        auto& item = _inventory[i];
        if (item.bombTimer < 0) continue;
        item.bombTimer -= 1;
        if (item.bombTimer <= 0) {
            _bombExpiredIndex = (int)i;
            _bombExpired = true;
            setHealth(_health - 2);
            break;
        }
    }
}

