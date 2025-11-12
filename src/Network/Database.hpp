#pragma once

#include "../lib/jsonlib.h"

/*
    Client's Database structure (JSON):
    {
        "type": "database",
        "users": {
            "@user": {
                "name": "user's real name (string)",
                "channels": [array with channels names],
                "contacts": {
                    "@username": "contact name (string)"
                },
                "email": "user's email (string)",
                "status": "active/inactive/banned (string)"
            }
            in client's database the user's count will be always 1
        }
    }
*/

class Database {
private:
    Json db;
    std::string username;
    using Key = const std::string&;
    using Str = const std::string&;
public:
    Database(const std::string& username) : username(username) {
        db = Json({
           {"type", "database"},
           {"users", Json({
            {username, Json({
                {"name", "NAME"},
                {"channels", Json::array()},
                {"contacts", Json::object()},
                {"email", "EMAIL"},
                {"status", "STATUS"}
            })}
           })}
        });
    }

    std::string getString() {
        return jsonToString(db);
    }

    void init(const Json& json) {
        db["users"][username]["name"] = json["name"];
        db["users"][username]["channels"] = json["channels"];
        db["users"][username]["contacts"] = json["contacts"];
        db["users"][username]["email"] = json["email"];
        db["users"][username]["status"] = json["status"];
    }

    bool is_channel_member(const std::string& channel) {
        if (channel[0] == '@' || channel[0] == '!') return true; // It's not a channel, it's a user or a special destination
        std::vector<std::string> channels = db["users"][username]["channels"];
        for (auto& i : channels) {
            if (i == channel) return true;
        }
        return false;
    }

    void join(const std::string& channel) {
        db["users"][username]["channels"].push_back(channel);
    }

    void leave(const std::string& channel) {
        std::vector<std::string> channels = db["users"][username]["channels"];
        db["users"][username]["channels"] = Json::array();
        for (auto& i : channels) {
            if (i != channel) {
                db["users"][username]["channels"].push_back(i);
            }
        }
    }

    void add_contact(Key username, Str contact) {
        db["users"][this->username]["contacts"][username] = contact;
    }

    void remove_contact(Key contact) {
        if (!db["users"][username]["contacts"].contains(contact)) return;
        auto items_copy = db["users"][username]["contacts"].items();
        db["users"][username]["contacts"] = Json::object();
        for (auto& [key, val] : items_copy) {
            if (key != contact) db["users"][username]["contacts"][key] = val;
        }
    }

    std::string contact(Key contact) {
        if (!db["users"][username]["contacts"].contains(contact)) return PULSAR_NO_CONTACT;

        return db["users"][username]["contacts"][contact];
    }
};