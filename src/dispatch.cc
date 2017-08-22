#include "client.h"

using namespace cordpp;

void Gateway::dispatch(const std::string &event, const Json &data) {
  std::cout << "[cordpp] Handling event: " << event << std::endl;

  if (event == "READY") {
    session_id = data["session_id"];
    
  } else if (event == "RESUMED") {

  } else if (event == "CHANNEL_CREATE") {
    
  } else if (event == "CHANNEL_UPDATE") {
    
  } else if (event == "CHANNEL_DELETE") {
    
  } else if (event == "CHANNEL_PINS_UPDATE") {
    
  } else if (event == "GUILD_CREATE") {
    
    
  } else if (event == "GUILD_UPDATE") {
    
  } else if (event == "GUILD_DELETE") {
    
  } else if (event == "GUILD_BAN_ADD") {
    
  } else if (event == "GUILD_BAN_REMOVE") {
    
  } else if (event == "GUILD_EMOJIS_UPDATE") {
    
  } else if (event == "GUILD_INTEGRATIONS_UPDATE") {
    
  } else if (event == "GUILD_MEMBER_ADD") {
    
  } else if (event == "GUILD_MEMBER_REMOVE") {
    
  } else if (event == "GUILD_MEMBER_UPDATE") {
    
  } else if (event == "GUILD_MEMBERS_CHUNK") {
    
  } else if (event == "GUILD_ROLE_CREATE") {
    
  } else if (event == "GUILD_ROLE_UPDATE") {
    
  } else if (event == "GUILD_ROLE_DELETE") {
    
  } else if (event == "MESSAGE_CREATE") {
    
  } else if (event == "MESSAGE_UPDATE") {
    
  } else if (event == "MESSAGE_DELETE") {
    
  } else if (event == "MESSAGE_DELETE_BULK") {
    
  } else if (event == "MESSAGE_REACTION_ADD") {
    
  } else if (event == "MESSAGE_REACTION_REMOVE") {
    
  } else if (event == "MESSAGE_REACTION_REMOVE_ALL") {
    
  } else if (event == "PRESENCE_UPDATE") {
    
  } else if (event == "USER_UPDATE") {
    
  } else if (event == "VOICE_STATE_UPDATE") {
    
  } else if (event == "VOICE_SERVER_UPDATE") {
    
  } else if (event == "WEBHOOKS_UPDATE") {
    
  }
}