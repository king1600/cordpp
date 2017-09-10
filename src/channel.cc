#include "items.h"
#include "channel.h"

using namespace cordpp;

Channel::Channel(Client *client) : Item() {
  this->client = client;
  this->_type = ChannelType::GuildText;
}

TextChannel::TextChannel(Client *c) : Channel(c) {
}

DMChannel::DMChannel(Client *c) : TextChannel(c) {
}

GuildTextChannel::GuildTextChannel(Client *c) : TextChannel(c) {
  overwrites.cache_size(10);
}

GuildVoiceChannel::GuildVoiceChannel(Client *c) : Channel(c)  {
  overwrites.cache_size(10);
}

ChannelType Channel::type() const {
  return _type;
}

const std::string GuildTextChannel::name() const {
  return _name;
}
const std::string GuildTextChannel::topic() const {
  return _topic;
}
const uint32_t GuildTextChannel::position() const {
  return _position;
}

const uint32_t GuildVoiceChannel::limit() const {
  return _user_limit;
}
const uint32_t GuildVoiceChannel::bitrate() const {
  return _bitrate;
}
const std::string GuildVoiceChannel::name() const {
  return _name;
}
const uint32_t GuildVoiceChannel::position() const {
  return _position;
}

void DMChannel::parse(Json &data) {
  
}

void GuildTextChannel::parse(Json &data) {
  
}

void GuildVoiceChannel::parse(Json &data) {
  
}