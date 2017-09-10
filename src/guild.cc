#include "guild.h"

using namespace cordpp;

Guild::Guild() : Item() {
  _roles.cache_size(10);
  _emojis.cache_size(10);
  _members.cache_size(10);
  _channels.cache_size(10);
}

void Guild::parse(Json& data) {

}

const std::string& Guild::name() const {
  return _name;
}
const std::string& Guild::icon() const {
  return _icon;
}
const std::string& Guild::splash() const {
  return _splash;
}
const std::string& Guild::region() const {
  return _region;
}

const bool& Guild::large() const {
  return _large;
}
const bool& Guild::unavailable() const {
  return _unavailable;
}
const bool& Guild::embed_enabled() const {
  return _embed_enabled;
}

const uint32_t& Guild::mfa_level() const {
  return _mfa_level;
}
const uint32_t& Guild::afk_timeout() const {
  return _afk_timeout;
}
const uint32_t& Guild::verify_level() const {
  return _verif_level;
}
const uint32_t& Guild::member_count() const {
  return _member_count;
}
const uint32_t& Guild::widget_enabled() const {
  return _widget_enabled;
}
const uint32_t& Guild::default_notifs() const {
  return _default_notifs;
}
const uint32_t& Guild::content_filter() const {
  return _content_filter;
}
const snowflake& Guild::application_id() const {
  return _application;
}

Collection<Role>& Guild::roles() {
  return _roles;
}
Collection<Emoji>& Guild::emojis() {
  return _emojis;
}
Collection<Member>& Guild::members() {
  return _members;
}
Collection<Channel>& Guild::channels() {
  return _channels; 
}

GuildVoiceChannel& Guild::afk_channel() {
  return *_afk_channel;
}
GuildTextChannel& Guild::embed_channel() {
  return *_embed_channel;
}
GuildTextChannel& Guild::widget_channel() {
  return *_widget_channel;
}
std::vector<std::string>& Guild::features() {
  return _features;
}
std::vector<VoiceState>& Guild::voice_states() {
  return _voice_states;
}