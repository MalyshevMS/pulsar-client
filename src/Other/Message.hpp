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
    Message(time_t datetime, const std::string& src, const std::string& dst, const std::string& msg)
     : datetime(datetime), src(src), dst(dst), msg(msg) {}

    time_t get_time() { return datetime; }
    std::string get_src() { return src; }
    std::string get_dst() { return dst; }
    std::string get_msg() { return msg; }

    Json to_json() const {
        return Json({
            {"time", datetime},
            {"src", src},
            {"dst", dst},
            {"msg", msg}
        });
    }

    Message& from_json(const Json& json) {
        datetime = json["time"];
        src = json["src"];
        dst = json["dst"];
        msg = json["msg"];

        return *this;
    }

    bool operator==(const Message& other) const {
        return (
            datetime == other.datetime && \
            src == other.src && \
            dst == other.dst && \
            msg == other.msg
        );
    }

    bool operator!=(const Message& other) const {
        return !(*(this) == other);
    }

    void set_id(size_t id) {
        this->id = id;
    }

    size_t get_id() const {
        return id;
    }

    friend std::ostream& operator<<(std::ostream& stream, Message& msg) {
        if (msg == PULSAR_NO_MESSAGE) return stream;
        stream << "[time: " << msg.datetime << " | from " << msg.src << " to " << msg.dst << "]: " << msg.msg;
        return stream;
    }

    friend class Database;
};

static Message parse_line(const std::string& line, const std::string& destination) {
    auto l_id = line.substr(0, PULSAR_ID_SIZE);
    auto l_time = line.substr(PULSAR_ID_SIZE, PULSAR_DATE_SIZE);
    auto l_src = line.substr(PULSAR_ID_SIZE + PULSAR_DATE_SIZE, PULSAR_USERNAME_SIZE);
    auto l_msg = line.substr(PULSAR_ID_SIZE + PULSAR_DATE_SIZE + PULSAR_USERNAME_SIZE, PULSAR_MESSAGE_SIZE);

    auto ret = Message(atoll(l_time.c_str()), l_src, destination, l_msg);
    ret.set_id(std::stoull(l_id));

    return ret;
}