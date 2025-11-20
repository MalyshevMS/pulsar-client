#pragma once

#include <string>
#include "Datetime.hpp"

class Profile {
private:
    std::string m_description;
    std::string m_email;
    std::string m_realName;
    Datetime m_birthday;
public:
    Profile(std::string description, std::string email, std::string realName, Datetime birthday)
     : m_description(description), m_email(email), m_realName(realName), m_birthday(birthday) {}

    // getters
    std::string description(void) const { return m_description; }
    std::string email(void) const { return m_email; }
    std::string realName(void) const { return m_realName; }
    Datetime birthday(void) const { return m_birthday; }

    // setters
    void description(const std::string& description) {
        m_description = description;
    }

    void email(const std::string& email) {
        m_email = email;
    }

    void realName(const std::string& realName) {
        m_realName = realName;
    }

    void birthday(const Datetime& birthday) {
        m_birthday = birthday;
    }


    static Profile fromJson(const Json& json) {
        Profile inst = PULSAR_NO_PROFILE;
        inst.m_description = json["description"];
        inst.m_email = json["email"];
        inst.m_realName = json["name"];
        inst.m_birthday = Datetime(json["birthday"]);

        return inst;
    }

    Json toJson() const {
        Json json;

        json["description"] = description();
        json["email"] = email();
        json["name"] = realName();
        json["birthday"] = birthday().toTime();

        return json;
    }
};