#pragma once

#include <ctime>
#include <string>

class Datetime {
private:
    time_t t_now = 0ull;
    struct tm* localTime = nullptr;
public:
    Datetime() {
        t_now = time(nullptr);
        localTime = localtime(&t_now);
    }

    void update() {
        t_now = time(nullptr);
        localTime = localtime(&t_now);
    }

    static Datetime now() {
        return Datetime();
    }

    constexpr const int year()  const { return localTime->tm_year + 1900; }
    constexpr const int month() const { return localTime->tm_mon + 1; }
    constexpr const int day()   const { return localTime->tm_mday; }
    constexpr const int hour()  const { return localTime->tm_hour; }
    constexpr const int min()   const { return localTime->tm_min; }
    constexpr const int sec()   const { return localTime->tm_sec; }

    std::string toString() const {
        return std::to_string(t_now);
    }

    time_t toTime() const {
        return t_now;
    }

    static Datetime fromString(const std::string& str) {
        auto dt = Datetime();
        dt.t_now = std::stoull(str);
        dt.localTime = localtime(&dt.t_now);
        return dt;
    }
};