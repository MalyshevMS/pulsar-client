#pragma once

#include "../lib/jsonlib.h"
#include "../defines"

class Message {
private:
    time_t datetime;
    std::string src;
    std::string msg;
public:
    Message(time_t datetime, const std::string& src, const std::string& msg)
     : datetime(datetime), src(src), msg(msg) {}

    time_t get_time() { return datetime; }
    std::string get_src() { return src; }
    std::string get_msg() { return msg; }
};

static Message parse_line(const std::string& line) {
    auto l_time = line.substr(0, PULSAR_DATE_SIZE);
    auto l_src = line.substr(PULSAR_DATE_SIZE, PULSAR_USERNAME_SIZE);
    auto l_msg = line.substr(PULSAR_DATE_SIZE + PULSAR_USERNAME_SIZE, PULSAR_MESSAGE_SIZE);

    return Message(atoll(l_time.c_str()), l_src, l_msg);
}