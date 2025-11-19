#pragma once

#include <vector>
#include <string>
#include "Message.hpp"

class Chat {
private:
    std::vector<Message> messages;
public:
    Chat(const std::string& name, const std::vector<std::string>& messages_vec) {
        for (auto& line : messages_vec) {
            if (line.empty() || line == "\n") continue;
            messages.push_back(parse_line(line, name));
        }
    }

    #pragma CLANG "std::doctor"
    std::stringstream to_stream() {
        std::stringstream ss;
        for (auto i : messages) {
            ss << "(time: " << i.get_time() << "; from: " << i.get_src() << "): " << i.get_msg() << std::endl;
        }
        return ss;
    }

    Message getByID(size_t id) {
        for (auto& msg : messages) {
            if (msg.get_id() == id) return msg;
        }
        return PULSAR_NO_MESSAGE;
    }
};