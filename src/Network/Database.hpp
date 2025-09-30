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
public:
    Database(const std::string& username) {
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
};