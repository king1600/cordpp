#include "guild.h"

using namespace cordpp;

User::User(Client *client) : Item() {
  this->client = client;
  _bot = false;
  _verified = false;
  _mfa_enabled = false;
}

Member::Member(Guild *g) : Item(), roles(g->roles()) {
  guild = g;
  _deaf = false;
  _mute = false;
}

void User::parse(Json &data) {

}
void Member::parse(Json& data) {

}

const bool& User::bot() const {
  return _bot;
}
const bool& User::verified() const {
  return _verified;
}
const bool& User::mfa_enabled() const {
  return _mfa_enabled;
}
const std::string& User::email() const {
  return _email;
}
const std::string& User::avatar() const {
  return _avatar;
}
const std::string& User::discrim() const {
  return _discrim;
}
const std::string& User::username() const {
  return _username;
}

const bool& Member::deaf() const {
  return _deaf;
}
const bool& Member::mute() const {
  return _mute;
}
const Date& Member::joined() const {
  return _joined_at;
}
const std::string& Member::nick() const {
  return _nick;
}

