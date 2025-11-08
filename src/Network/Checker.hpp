//  === Pulsar Checker version 0.0.1 ===
//  Last update/sync: 08.11.2025 23:45 (UTC+3 Moscow)
//  This header must be synchronized BOTH the server and client
#pragma once

#include "../defines"
#include <string>

#define PULSAR_CHECKER_VERSION "0.0.1"

namespace Checker {
    // Base Checker exception class
    class exception : public std::exception {
    private:
        std::runtime_error m;
    public:
        exception(const std::string& error_message) : m("Pulsar Checker v" + std::string(PULSAR_CHECKER_VERSION) + " exception: " + error_message) {};
        
        const char* what() const noexcept override {
            return m.what();
        }
    };

    // Other exceptions
    class UsernameFailed : public exception {
    public:
        UsernameFailed(const std::string& u = "!NONE") : exception("Username check failed for '" + u + "'") {};
    };

    class ChannelNameFailed : public exception {
    public:
        ChannelNameFailed(const std::string& c = "!NONE") : exception("Channel name check failed for '" + c + "'") {};
    };

    /// @return Current version of Checker
    /// @warning If this doesn't matches the server version, the client should be closed immediatly
    const static std::string version() { return PULSAR_CHECKER_VERSION; }

    // Username
    using Username = const std::string&;
    char username_blocked_symbols[] = {
        ' ', '!', '#', '$', '%', '^', '&', '*', '(', ')',
        '-', '=', '+', '[', ']', '{', '}', '`', '~', '\047',
        '"', '<', '>', '?', ',', '.', '/', '|', '\\', ':', ';'
    };

    std::string username_blocked[] = {
        "@admin", "@browser", "@server", "@all"
    };

    bool checkUsername(Username username) {
        if (username[0] != '@') return false;
        if (username.size() > PULSAR_USERNAME_SIZE) return false;

        for (int i = 0; i < sizeof(username_blocked_symbols) / sizeof(char); i++) {
            if (username.find(username_blocked_symbols[i]) != std::string::npos) return false;
        }

        for (int i = 0; i < sizeof(username_blocked) / sizeof(std::string); i++) {
            if (username == username_blocked[i]) return false;
        }

        for (auto i : username) {
            if (i != tolower(i)) return false;
        }

        return true;
    }

    // Channels
    using Channel = const std::string&;
    char channel_blocked_symbols[] = {
        ' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '-', '=', '+', '[', ']', '{', '}', '`', '~', '\047',
        '"', '<', '>', '?', ',', '.', '/', '|', '\\', ';'
    };

    std::string channel_blocked[] = {
        ":all", ":server", ":browser"
    };

    bool checkChannelName(Channel channel) {
        if (channel[0] != ':') return false;
        if (channel.size() > PULSAR_USERNAME_SIZE) return false;

        for (int i = 0; i < sizeof(channel_blocked_symbols) / sizeof(char); i++) {
            if (channel.find(channel_blocked_symbols[i]) != std::string::npos) return false;
        }

        for (int i = 0; i < sizeof(channel_blocked) / sizeof(std::string); i++) {
            if (channel == channel_blocked[i]) return false;
        }

        for (auto i : channel) {
            if (i != tolower(i)) return false;
        }

        return true;
    }
};

#define PULSAR_THROW throw Checker::