#include "items.h"

using namespace cordpp;

Overwrite::Overwrite() : Item() {
  _deny = 0;
  _allow = 0;
}

Role::Role(Guild *g) : Item() {
  guild = g;
  _color = 0;
  _position = 0;
  _hoist = false;
  _managed = false;
  _permissions = 0;
  _mentionable = false;
}


Emoji::Emoji(Guild *g) : Item()  {
  guild = g;
  _managed = false;
  _roles.cache_size(2);
  _require_colons = false;
}

const snowflake Item::id() {
  return _id;
}

void Item::parse(Json &data) {
  _id = data["id"].get<snowflake>();
}

void Role::parse(Json& data) {

}

void Emoji::parse(Json& data) {
  
}

void Overwrite::parse(Json& data) {

}

const uint32_t& Overwrite::deny() const {
  return _deny;
}
const uint32_t& Overwrite::allow() const {
  return _allow;
}
const std::string& Overwrite::type() const {
  return _type;
}

const bool& Role::hoist() const {
  return _hoist;
}
const bool& Role::managed() const {
  return _managed;
}
const uint32_t& Role::color() const {
  return _color;
}
const bool& Role::mentionable() const {
  return _mentionable;
}
const uint32_t& Role::position() const {
  return _position;
}
const uint32_t& Role::permissions() const {
  return _permissions;
}

const bool& Emoji::managed() const {
  return _managed;
}
const std::string& Emoji::name() const {
  return _name;
}
const bool& Emoji::require_colons() const {
  return _require_colons;
}
const Collection<Role>& Emoji::roles() const {
  return _roles;
}

