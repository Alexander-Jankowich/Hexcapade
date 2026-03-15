#ifndef __MESSAGE_SERIALIZER_H__
#define __MESSAGE_SERIALIZER_H__

#include <cugl/cugl.h>
#include "../Models/SpellSet.h"

struct ClientInput
{
    float joystickAngle = 0.0f;
    SpellType spellType = SpellType::NONE;
    float spellAngle = 0.0f;
    std::string sid = "_";
    bool pickupPressed = false;
    bool dropPressed = false;
    bool depositPressed = false;
    int dropIndex = -1;
    float posX = 0.0f;
    float posY = 0.0f;
};

struct HostInventoryItemData
{
    std::string id;
    int kind = 0;
    std::string name;
    int value = 0;
};

struct HostWorldItemData
{
    std::string id;
    int kind = 0;
    std::string name;
    int value = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    float radius = 0.0f;
};

struct HostPlayerData
{
    std::string uuid;
    float posX = 0.0f;
    float posY = 0.0f;
    float speed = 0.0f;
    float velAngle = 0.0f;
    int inventoryVal = 0;
    SpellType curSpell = SpellType::NONE;
    int health = 12;
    std::vector<HostInventoryItemData> inventoryItems;
};

struct HostEnemyData
{
    float posX = 0.0f;
    float posY = 0.0f;
    float speed = 0.0f;
    float velAngle = 0.0f;
    int health = 0;
    int eid = 0;
};

struct HostSpellData
{
    std::string id;
    float posX = 0.0f;
    float posY = 0.0f;
    float velAngle = 0.0f;
    SpellType type = SpellType::NONE;
};

struct HostInput
{
    float dt = 0.0f;
    int depositedValue = 0;
    int quota = 0;
    // ga: procnet
    // host-chosen run metadata so every client can rebuild the same map
    int runRevision = 0;
    unsigned int mapSeed = 0;
    float mapSceneWidth = 0.0f;
    float mapSceneHeight = 0.0f;
    std::vector<HostPlayerData> players;
    std::vector<HostEnemyData> enemies;
    std::vector<HostSpellData> spells;
    std::vector<HostWorldItemData> worldItems;

    // GA: itemnet old delta path retained only as a note during migration.
    // std::vector<HostItemData> added_items;
    // std::vector<int> removed_items;
};

class MessageSerializer
{
public:
    static std::vector<std::byte> encodeClientInput(const ClientInput& input)
    {
        cugl::netcode::NetcodeSerializer s;
        s.writeFloat(input.joystickAngle);
        s.writeSint32(spellTypeToInt(input.spellType));
        s.writeFloat(input.spellAngle);
        s.writeString(input.sid);
        s.writeBool(input.pickupPressed);
        s.writeBool(input.dropPressed);
        s.writeBool(input.depositPressed);
        s.writeSint32(input.dropIndex);
        s.writeFloat(input.posX);
        s.writeFloat(input.posY);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static bool decodeClientInput(const std::vector<std::byte>& data, ClientInput& output)
    {
        if (data.empty()) {
            return false;
        }

        cugl::netcode::NetcodeDeserializer d;
        d.receive(data);
        output.joystickAngle = d.readFloat();
        output.spellType = intToSpellType(d.readSint32());
        output.spellAngle = d.readFloat();
        try {
            output.sid = d.readString();
        }
        catch (std::bad_variant_access) {
            output.sid = "_";
        }
        output.pickupPressed = d.readBool();
        output.dropPressed = d.readBool();
        output.depositPressed = d.readBool();
        output.dropIndex = d.readSint32();
        output.posX = d.readFloat();
        output.posY = d.readFloat();
        return true;
    }

    static std::vector<std::byte> encodeHostStartGame(const std::vector<HostPlayerData>& input)
    {
        cugl::netcode::NetcodeSerializer s;
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendArray("players");
        auto playersArr = json->get("players");
        for (const HostPlayerData& p : input)
        {
            playersArr->appendChild(playerDataToJson(p));
        }
        s.writeJson(json);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static std::vector<std::byte> encodeHostInput(const HostInput& input)
    {
        cugl::netcode::NetcodeSerializer s;
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("dt", (double)input.dt);
        json->appendValue("deposited_value", (long)input.depositedValue);
        json->appendValue("quota", (long)input.quota);
        json->appendValue("run_revision", (long)input.runRevision);
        json->appendValue("map_seed", (long)input.mapSeed);
        json->appendValue("map_scene_width", (double)input.mapSceneWidth);
        json->appendValue("map_scene_height", (double)input.mapSceneHeight);

        json->appendArray("players");
        auto playersArr = json->get("players");
        for (const HostPlayerData& p : input.players)
        {
            playersArr->appendChild(playerDataToJson(p));
        }

        json->appendArray("enemies");
        auto enemiesArr = json->get("enemies");
        for (const HostEnemyData& e : input.enemies)
        {
            enemiesArr->appendChild(enemyDataToJson(e));
        }

        json->appendArray("spells");
        auto spellsArr = json->get("spells");
        for (const HostSpellData& e : input.spells)
        {
            spellsArr->appendChild(spellDataToJson(e));
        }

        json->appendArray("world_items");
        auto worldItemsArr = json->get("world_items");
        for (const HostWorldItemData& e : input.worldItems)
        {
            worldItemsArr->appendChild(worldItemDataToJson(e));
        }

        s.writeJson(json);
        std::vector<std::byte> vec = s.serialize();
        s.reset();
        return vec;
    }

    static bool decodeHostInput(const std::vector<std::byte>& data, HostInput& output)
    {
        if (data.empty()) {
            return false;
        }

        cugl::netcode::NetcodeDeserializer d;
        d.receive(data);
        if (d.nextType() != cugl::netcode::NetcodeType::JsonType) {
            return false;
        }

        std::shared_ptr<cugl::JsonValue> json = d.readJson();

        output.dt = json->getFloat("dt");
        output.depositedValue = json->getInt("deposited_value");
        output.quota = json->getInt("quota");
        output.runRevision = json->getInt("run_revision", output.runRevision);
        output.mapSeed = (unsigned int)json->getInt("map_seed", (int)output.mapSeed);
        output.mapSceneWidth = json->getFloat("map_scene_width", output.mapSceneWidth);
        output.mapSceneHeight = json->getFloat("map_scene_height", output.mapSceneHeight);

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

        std::vector<HostWorldItemData> wdata;
        for (auto c : json->get("world_items")->children())
        {
            wdata.push_back(jsonToWorldItemData(c));
        }
        output.worldItems = wdata;
        return true;
    }

private:
    static int spellTypeToInt(SpellType type)
    {
        switch (type)
        {
        case SpellType::FIREBALL:
            return 1;
        case SpellType::FREEZE:
            return 2;
        case SpellType::HEAL:
            return 3;
        case SpellType::NONE:
        default:
            return 0;
        }
    }

    static SpellType intToSpellType(int32_t value)
    {
        switch (value)
        {
        case 1:
            return SpellType::FIREBALL;
        case 2:
            return SpellType::FREEZE;
        case 3:
            return SpellType::HEAL;
        default:
            return SpellType::NONE;
        }
    }

    static std::shared_ptr<cugl::JsonValue> inventoryItemDataToJson(const HostInventoryItemData& data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("id", data.id);
        json->appendValue("kind", (long)data.kind);
        json->appendValue("name", data.name);
        json->appendValue("value", (long)data.value);
        return json;
    }

    static HostInventoryItemData jsonToInventoryItemData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostInventoryItemData res;
        res.id = json->getString("id");
        res.kind = json->getInt("kind");
        res.name = json->getString("name");
        res.value = json->getInt("value");
        return res;
    }

    static std::shared_ptr<cugl::JsonValue> worldItemDataToJson(const HostWorldItemData& data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("id", data.id);
        json->appendValue("kind", (long)data.kind);
        json->appendValue("name", data.name);
        json->appendValue("value", (long)data.value);
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("radius", (double)data.radius);
        return json;
    }

    static HostWorldItemData jsonToWorldItemData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostWorldItemData res;
        res.id = json->getString("id");
        res.kind = json->getInt("kind");
        res.name = json->getString("name");
        res.value = json->getInt("value");
        res.posX = json->getFloat("pos_x");
        res.posY = json->getFloat("pos_y");
        res.radius = json->getFloat("radius");
        return res;
    }

    static std::shared_ptr<cugl::JsonValue> playerDataToJson(const HostPlayerData& data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("uuid", data.uuid);
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("speed", (double)data.speed);
        json->appendValue("vel_angle", (double)data.velAngle);
        json->appendValue("inventory_val", (long)data.inventoryVal);
        json->appendValue("cur_spell", (long)spellTypeToInt(data.curSpell));
        json->appendValue("health", (long)data.health);

        json->appendArray("inventory_items");
        auto inventoryArr = json->get("inventory_items");
        for (const HostInventoryItemData& item : data.inventoryItems)
        {
            inventoryArr->appendChild(inventoryItemDataToJson(item));
        }

        return json;
    }

    static HostPlayerData jsonToPlayerData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostPlayerData res;
        res.posX = json->getFloat("pos_x");
        res.posY = json->getFloat("pos_y");
        res.speed = json->getFloat("speed");
        res.velAngle = json->getFloat("vel_angle");
        res.inventoryVal = json->getInt("inventory_val");
        res.curSpell = intToSpellType(json->getInt("cur_spell"));
        res.health = json->getInt("health");
        res.uuid = json->getString("uuid");

        std::shared_ptr<cugl::JsonValue> inventoryItems = json->get("inventory_items");
        if (inventoryItems) {
            for (auto c : inventoryItems->children())
            {
                res.inventoryItems.push_back(jsonToInventoryItemData(c));
            }
        }
        return res;
    }

    static std::shared_ptr<cugl::JsonValue> enemyDataToJson(const HostEnemyData& data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("speed", (double)data.speed);
        json->appendValue("vel_angle", (double)data.velAngle);
        json->appendValue("health", (long)data.health);
        json->appendValue("eid", (long)data.eid);
        return json;
    }

    static HostEnemyData jsonToEnemyData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostEnemyData res;
        res.posX = json->getFloat("pos_x");
        res.posY = json->getFloat("pos_y");
        res.speed = json->getFloat("speed");
        res.velAngle = json->getFloat("vel_angle");
        res.health = json->getInt("health");
        res.eid = json->getInt("eid");
        return res;
    }

    static std::shared_ptr<cugl::JsonValue> spellDataToJson(const HostSpellData& data)
    {
        std::shared_ptr<cugl::JsonValue> json = cugl::JsonValue::allocObject();
        json->appendValue("id", data.id);
        json->appendValue("pos_x", (double)data.posX);
        json->appendValue("pos_y", (double)data.posY);
        json->appendValue("vel_angle", (double)data.velAngle);
        json->appendValue("spell_type", (long)spellTypeToInt(data.type));
        return json;
    }

    static HostSpellData jsonToSpellData(const std::shared_ptr<cugl::JsonValue> json)
    {
        HostSpellData res;
        res.id = json->getString("id");
        res.posX = json->getFloat("pos_x");
        res.posY = json->getFloat("pos_y");
        res.velAngle = json->getFloat("vel_angle");
        res.type = intToSpellType(json->getInt("spell_type"));
        return res;
    }
};

#endif /* __MESSAGE_SERIALIZER_H__ */
