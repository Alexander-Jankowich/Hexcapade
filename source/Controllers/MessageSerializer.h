#ifndef __MESSAGE_SERIALIZER_H__
#define __MESSAGE_SERIALIZER_H__

#include <cugl/cugl.h>
#include "../Models/SpellSet.h"

struct ClientInput
{
    float joystickAngle = 0.0f;
    SpellType spellType = SpellType::NONE; // 0=NONE,1=FIREBALL,2=FREEZE
    float spellAngle = 0.0f;
    bool pickupPressed = false;
    float posX = 0.0f;
    float posY = 0.0f;
};

struct HostPlayerData
{
    std::string uuid;
    float posX=0.0f;
    float posY=0.0f;
    float speed=0.0f;
    float velAngle=0.0f;
    int inventoryVal=0;
    int health=12; // i think this is starting val?? will not be used tho
};
struct HostEnemyData
{
    float posX;
    float posY;
    float speed;
    float velAngle;
    int health;
    int eid;
};
struct HostSpellData
{
    float posX;
    float posY;
    float speed;
    float velAngle;
    SpellType type;
};
struct HostItemData
{
    float posX;
    float posY;
    int itemId;
    std::string item;
};
struct HostInput
{
    float dt;
    std::vector<HostPlayerData> players;
    std::vector<HostEnemyData> enemies;
    std::vector<HostSpellData> spells;
    std::vector<HostItemData> added_items;
    std::vector<int> removed_items;
};
class MessageSerializer
{
public:
    static std::vector<std::byte> encodeClientInput(const ClientInput &input)
    {
        cugl::netcode::NetcodeSerializer s;
        s.writeFloat(input.joystickAngle);
        switch (input.spellType)
        {
        case SpellType::FIREBALL:
            s.writeSint32(1);
            break;
        case SpellType::FREEZE:
            s.writeSint32(2);
            break;
        case SpellType::NONE:
            s.writeSint32(0);
            break;
        }
        s.writeFloat(input.spellAngle);
        s.writeBool(input.pickupPressed);
        s.writeFloat(input.posX);
        s.writeFloat(input.posY);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static bool decodeClientInput(const std::vector<std::byte> &data, ClientInput &output)
    {
        if (data.empty())
            return false;
        cugl::netcode::NetcodeDeserializer d;
        d.receive(data);
        output.joystickAngle = d.readFloat();
        int st = d.readSint32();
        switch (st) {
            case 1:
                output.spellType=SpellType::FIREBALL;
                break;
            case 2:
                output.spellType=SpellType::FREEZE;
                break;
            default:
                output.spellType=SpellType::NONE;
                break;
        }
        output.spellAngle = d.readFloat();
        output.pickupPressed = d.readBool();
        output.posX = d.readFloat();
        output.posY = d.readFloat();
        return true;
    }
    
    static std::vector<std::byte> encodeHostStartGame(const std::vector<HostPlayerData> &input) {
        cugl::netcode::NetcodeSerializer s;
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendArray("players");
        for (auto p : input)
        {
            json->appendChild(playerDataToJson(p));
        }
        s.writeJson(json);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static std::vector<std::byte> encodeHostInput(const HostInput &input)
    {
        cugl::netcode::NetcodeSerializer s;
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendArray("players");
        auto playersArr = json->get("players");
        for (auto p : input.players)
        {
            playersArr->appendChild(playerDataToJson(p));
        }
        json->appendArray("enemies");
        auto enemiesArr = json->get("enemies");
        for (auto e : input.enemies)
        {
            enemiesArr->appendChild(enemyDataToJson(e));
        }
        json->appendArray("spells");
        auto spellsArr = json->get("spells");
        for (auto e : input.spells)
        {
            spellsArr->appendChild(spellDataToJson(e));
        }
        json->appendArray("added_items");
        auto addedArr = json->get("added_items");
        for (auto e : input.added_items)
        {
            addedArr->appendChild(itemDataToJson(e));
        }
        json->appendArray("removed_items");
        auto removedArr = json->get("removed_items");
        for (auto e : input.removed_items)
        {
            std::shared_ptr<cugl::JsonValue> child = cugl::JsonValue::allocObject();
            child->appendValue("item_id", (long)e);
            removedArr->appendChild(child); // have to make child for deserialization purposes i think
        }
        s.writeJson(json);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static bool decodeHostInput(const std::vector<std::byte> &data, HostInput &output)
    {
        if (data.empty())
            return false;
        cugl::netcode::NetcodeDeserializer d;
        d.receive(data);
        if (d.nextType() != cugl::netcode::NetcodeType::JsonType)
            return false;

        std::shared_ptr<cugl::JsonValue> json = d.readJson();

        std::vector<HostPlayerData> pdata;
        for (auto c : json->get("players")->children())
        {
            pdata.push_back(jsonToPlayerData(c));
        }
        output.players = pdata;

        std::vector<HostEnemyData> edata;
        for (auto c : json->get("enemies")->children())
        {
            edata.push_back(jsonToEnemyData(c));
        }
        output.enemies = edata;

        std::vector<HostSpellData> sdata;
        for (auto c : json->get("spells")->children())
        {
            sdata.push_back(jsonToSpellData(c));
        }
        output.spells = sdata;

        std::vector<HostItemData> idata;
        for (auto c : json->get("added_items")->children())
        {
            idata.push_back(jsonToItemData(c));
        }
        output.added_items = idata;

        std::vector<int> rem_idata;
        for (auto c : json->get("removed_items")->children())
        {
            rem_idata.push_back(c->getInt("item_id"));
        }
        output.removed_items = rem_idata;
        return true;
    }

private:
    static std::shared_ptr<cugl::JsonValue> playerDataToJson(const HostPlayerData &data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("uuid", data.uuid);
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("speed", (double)data.speed);
        json->appendValue("vel_angle", (double)data.velAngle);
        json->appendValue("inventory_val", (long)data.inventoryVal);
        json->appendValue("health", (long)data.health);
        return json;
    }

    static HostPlayerData jsonToPlayerData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostPlayerData *res = new HostPlayerData();
        res->posX = json->getFloat("pos_x");
        res->posY = json->getFloat("pos_y");
        res->speed = json->getFloat("speed");
        res->velAngle = json->getFloat("vel_angle");
        res->inventoryVal = json->getInt("inventory_val");
        res->health = json->getInt("health");
        res->uuid = json->getString("uuid");
        return *res;
    }

    static std::shared_ptr<cugl::JsonValue> enemyDataToJson(const HostEnemyData &data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("speed", (double)data.speed);
        json->appendValue("vel_angle", (double)data.velAngle);
        json->appendValue("health", (long)data.health);
        json->appendValue("eid", (long) data.eid);
        return json;
    }

    static HostEnemyData jsonToEnemyData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostEnemyData *res = new HostEnemyData();
        res->posX = json->getFloat("pos_x");
        res->posY = json->getFloat("pos_y");
        res->speed = json->getFloat("speed");
        res->velAngle = json->getFloat("vel_angle");
        res->health = json->getInt("health");
        res->eid = json->getInt("eid");
        return *res;
    }

    static std::shared_ptr<cugl::JsonValue> spellDataToJson(const HostSpellData &data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("speed", (double)data.speed);
        json->appendValue("vel_angle", (double)data.velAngle);
        switch (data.type)
        {
        case SpellType::FIREBALL:
            json->appendValue("spell_type", (long)1);
            break;
        case SpellType::FREEZE:
            json->appendValue("spell_type", (long)2);
            break;
        case SpellType::NONE:
            json->appendValue("spell_type", (long)-1);
            break;
        }
        return json;
    }

    static HostSpellData jsonToSpellData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostSpellData *res = new HostSpellData();
        res->posX = json->getFloat("pos_x");
        res->posY = json->getFloat("pos_y");
        res->speed = json->getFloat("speed");
        res->velAngle = json->getFloat("vel_angle");
        switch (json->getInt("spell_type"))
        {
        case 1:
            res->type = SpellType::FIREBALL;
            break;
        case 2:
            res->type = SpellType::FREEZE;
            break;
        case -1:
            res->type = SpellType::NONE;
            break;
        }
        return *res;
    }

    static std::shared_ptr<cugl::JsonValue> itemDataToJson(const HostItemData &data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("item_id", (long)data.itemId);
        json->appendValue("item", data.item);
        return json;
    }
    static HostItemData jsonToItemData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostItemData *res = new HostItemData();
        res->posX = json->getFloat("pos_x");
        res->posY = json->getFloat("pos_y");
        res->itemId = json->getInt("item_id");
        res->item = json->getString("item");
        return *res;
    }
};

#endif /* __MESSAGE_SERIALIZER_H__ */
