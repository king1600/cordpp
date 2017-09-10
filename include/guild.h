#pragma once

#include "user.h"

namespace cordpp {

  class VoiceState {
  public:
    bool deaf = false;
    bool mute = false;
    User *user = nullptr;
    bool suppress = false;
    bool self_deaf = false;
    bool self_mute = false;
    std::string session_id;
    Guild *guild = nullptr;
    GuildVoiceChannel *channel = nullptr;
  };

  class Client;
  class Guild : public Item {
  private:
    Date _joined;
    Member *_owner;
    std::string _name;
    std::string _icon;
    std::string _splash;
    std::string _region;

    bool _large;
    bool _unavailable;
    bool _embed_enabled;

    uint32_t _mfa_level;
    uint32_t _afk_timeout;
    uint32_t _verif_level;
    snowflake _application;
    uint32_t _member_count;
    uint32_t _widget_enabled;
    uint32_t _default_notifs;
    uint32_t _content_filter;

    Collection<Role> _roles;
    Collection<Emoji> _emojis;
    Collection<Member> _members;
    Collection<Channel> _channels;
    GuildVoiceChannel *_afk_channel;
    GuildTextChannel *_embed_channel;
    GuildTextChannel *_widget_channel;
    std::vector<std::string> _features;
    std::vector<VoiceState> _voice_states;

  public:
    Guild();
    void parse(Json& data);
    const std::string& name() const;
    const std::string& icon() const;
    const std::string& splash() const;
    const std::string& region() const;

    const bool& large() const;
    const bool& unavailable() const;
    const bool& embed_enabled() const;

    const uint32_t& mfa_level() const;
    const uint32_t& afk_timeout() const;
    const uint32_t& verify_level() const;
    const uint32_t& member_count() const;
    const uint32_t& widget_enabled() const;
    const uint32_t& default_notifs() const;
    const uint32_t& content_filter() const;
    const snowflake& application_id() const;

    Collection<Role>& roles();
    Collection<Emoji>& emojis();
    Collection<Member>& members();
    Collection<Channel>& channels();
    GuildVoiceChannel& afk_channel();
    GuildTextChannel& embed_channel();
    GuildTextChannel& widget_channel();
    std::vector<std::string>& features();
    std::vector<VoiceState>& voice_states();
  };
}