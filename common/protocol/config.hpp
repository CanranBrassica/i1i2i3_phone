#pragma once

struct gateway_server_config
{
    const char* ip_addr;
    int port;
};

struct voice_server_config
{
    const char* ip_addr;
    int port;
};

struct client_config
{
    const char* ip_addr;
    int port;
};
