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

    time_t get_time() const { return datetime; }
    std::string get_src() const { return src; }
    std::string get_dst() const { return dst; }
    std::string get_msg() const { return msg; }

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

    std::string to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& stream, const Message& message) {
        Message out = message;
        if (out == PULSAR_NO_MESSAGE) return stream;
        if (out.msg.back() == '\n') out.msg.pop_back();
        stream << "(" << out.id << ")[" << Datetime(out.datetime).toFormattedString() << " от " << out.src << " в " << out.dst << "]: " << out.msg;
        return stream;
    }

    friend class Database;
};

static Message parse_line(const std::string& line, const std::string& destination) {
    auto l_id = line.substr(0, PULSAR_ID_SIZE);
    auto l_time = line.substr(PULSAR_ID_SIZE, PULSAR_DATE_SIZE);
    auto l_src = line.substr(PULSAR_ID_SIZE + PULSAR_DATE_SIZE, line.find(PULSAR_SEP) - (PULSAR_ID_SIZE + PULSAR_DATE_SIZE));
    auto l_msg = line.substr(line.find(PULSAR_SEP) + 1);

    return Message(std::stoull(l_id), atoll(l_time.c_str()), l_src, destination, l_msg);
}