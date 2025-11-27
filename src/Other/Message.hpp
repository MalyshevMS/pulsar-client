#pragma once

#include "../lib/jsonlib.h"
#include "../defines"

class Message {
private:
    size_t id;
    time_t datetime;
    std::string src;
    std::string msg;
    std::string dst;
public:
    Message(size_t id, time_t datetime, const std::string& src, const std::string& dst, const std::string& msg)
     : id(id), datetime(datetime), src(src), dst(dst), msg(msg) {}

    time_t get_time() { return datetime; }
    std::string get_src() { return src; }
    std::string get_dst() { return dst; }
    std::string get_msg() { return msg; }

    Json to_json() const {
        return Json({
            {"time", datetime},
            {"id", id},
            {"src", src},
            {"dst", dst},
            {"msg", msg}
        });
    }

    Message& from_json(const Json& json) {
        id = json["id"];
        datetime = json["time"];
        src = json["src"];
        dst = json["dst"];
        msg = json["msg"];

        return *this;
    }

    bool operator==(const Message& other) const {
        return (
            id == other.id && \
            datetime == other.datetime && \
            src == other.src && \
            dst == other.dst && \
            msg == other.msg
        );
    }

    bool operator!=(const Message& other) const {
        return !(*(this) == other);
    }

    size_t get_id() const {
        return id;
    }

    friend std::ostream& operator<<(std::ostream& stream, Message& msg) {
        if (msg == PULSAR_NO_MESSAGE) return stream;
        stream << "(" << msg.id << ")[" << Datetime(msg.datetime).toFormattedString() << " от " << msg.src << " в " << msg.dst << "]: " << msg.msg;
        return stream;
    }

    friend class Database;
};

static Message parse_line(const std::string& line, const std::string& destination) {
    auto l_id = line.substr(0, PULSAR_ID_SIZE);
    auto l_time = line.substr(PULSAR_ID_SIZE, PULSAR_DATE_SIZE);
    auto l_src = line.substr(PULSAR_ID_SIZE + PULSAR_DATE_SIZE, PULSAR_USERNAME_SIZE);
    auto l_msg = line.substr(PULSAR_ID_SIZE + PULSAR_DATE_SIZE + PULSAR_USERNAME_SIZE, PULSAR_MESSAGE_SIZE);
    try {
        size_t id = 0;
        time_t t = 0;
        try {
            id = std::stoull(l_id);
        } catch (...) {
            return PULSAR_NO_MESSAGE;
        }

        // atoll is tolerant, but try to parse safely as unsigned long long
        try {
            t = static_cast<time_t>(std::stoull(l_time));
        } catch (...) {
            // fallback to atoll which returns 0 on error
            t = atoll(l_time.c_str());
        }

        return Message(id, t, l_src, destination, l_msg);
    } catch (...) {
        return PULSAR_NO_MESSAGE;
    }
}