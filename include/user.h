#pragma once

#include "channel.h"
#include "date.h"

namespace cordpp {

  class Client;
  class User : public Item {
  private:
    bool _bot;
    bool _verified;
    bool _mfa_enabled;
    std::string _email;
    std::string _avatar;
    std::string _discrim;
    std::string _username;

  public:
    Client *client;
    User(Client *client = nullptr);

    void parse(Json &data);
    const bool& bot() const;
    const bool& verified() const;
    const bool& mfa_enabled() const;
    const std::string& email() const;
    const std::string& avatar() const;
    const std::string& discrim() const;
    const std::string& username() const;
  };

  class Guild;
  class Member : public Item {
  private:
    bool _deaf;
    bool _mute;
    Date _joined_at;
    std::string _nick;
  public:
    User *user;
    Guild *guild;
    Collection<Role> &roles;
    Member(Guild *guild);
    
    void parse(Json& data);
    const bool& deaf() const;
    const bool& mute() const;
    const Date& joined() const;
    const std::string& nick() const;
  };
}