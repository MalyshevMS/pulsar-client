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
        // TODO: implement this
        db["users"][username]["name"] = json["name"];
        db["users"][username]["channels"] = json["channels"];
        db["users"][username]["contacts"] = json["contacts"];
        db["users"][username]["email"] = json["email"];
        db["users"][username]["status"] = json["status"];
    }

    bool is_channel_member(const std::string& channel) {
        std::vector<std::string> channels = db["users"][username]["channels"];
        for (auto& i : channels) {
            if (i == channel) return true;
        }
        return false;
    }
};