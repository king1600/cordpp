#pragma once

#include "items.h"

namespace cordpp {

  enum ChannelType : uint8_t {
    GuildText = 0, DM = 1, GuildVoice = 2, GroupDM = 3, GuildCategory = 4
  };

  class Client;
  class Channel : public Item {
  private:
    ChannelType _type;
  public:
    Client *client;
    Channel(Client *client = nullptr);
    ChannelType type() const;
  };

  class TextChannel : public Channel {
  public:
    TextChannel(Client *client = nullptr);
  };

  class DMChannel : public TextChannel {
  public:
    DMChannel(Client *client = nullptr);
    void parse(Json &data);
  };

  class GuildTextChannel : public TextChannel {
  private:
    std::string _name;
    std::string _topic;
    uint32_t _position;

  public:
    Collection<Overwrite> overwrites;
    GuildTextChannel(Client *client = nullptr);
    
    void parse(Json& data);
    const std::string name() const;
    const std::string topic() const;
    const uint32_t position() const;
  };

  class GuildVoiceChannel : public Channel {
  private:
    uint32_t _bitrate;
    std::string _name;
    uint32_t _position;
    uint32_t _user_limit;

  public:
    Collection<Overwrite> overwrites;
    GuildVoiceChannel(Client *client = nullptr);
    
    void parse(Json& data);
    const uint32_t limit() const;
    const uint32_t bitrate() const;
    const std::string name() const;
    const uint32_t position() const;
  };

}