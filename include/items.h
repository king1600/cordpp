#pragma once

#include "collection.h"

namespace cordpp {

  class Item {
  private:
    snowflake _id;
  public:
    const snowflake id();
    void parse(Json& data);
  };

  class Overwrite : public Item {
  private:
    uint32_t _allow;
    uint32_t _deny;
    std::string _type;
  public:
    Overwrite();
    void parse(Json &data);
    const uint32_t& deny() const;
    const uint32_t& allow() const;
    const std::string& type() const;
  };

  class Guild;
  class Role : public Item {
  private:
    bool _hoist;
    bool _managed;
    uint32_t _color;
    bool _mentionable;
    std::string _name;
    uint32_t _position;
    uint32_t _permissions;
    
  public:
    Guild *guild;
    Role(Guild *guild = nullptr);
    void parse(Json& data);
    const bool& hoist() const;
    const bool& managed() const;
    const uint32_t& color() const;
    const bool& mentionable() const;
    const uint32_t& position() const;
    const uint32_t& permissions() const;
  };

  class Emoji : public Item {
  private:
    bool _managed;
    std::string _name;
    bool _require_colons;
    Collection<Role> _roles;
    
  public:
    Guild *guild;
    Emoji(Guild *guild = nullptr);
    void parse(Json& data);
    const bool& managed() const;
    const std::string& name() const;
    const bool& require_colons() const;
    const Collection<Role>& roles() const;
  };

}