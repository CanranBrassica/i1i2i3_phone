#pragma once

#include "client_agent.hpp"

#include <forward_list>

namespace IpPhone
{

struct VoiceChatRoom
{


    std::forward_list<std::weak_ptr<ClientAgent>> member;
};

}  // namespace IpPhone
