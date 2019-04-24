#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "route_handler.h"
#include "logger.h"

class Reverse_proxy_handler : public route_handler
{
public:
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
    // static std::unordered_map<std::string, std::string> get_proxies (std::shared_ptr<NginxConfig> config);
private:
    Reverse_proxy_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::string sendGetRequest(char* host, char* path);
    std::string constructGetRequest(std::string uri);
    char* extractRedirectedHost(std::string res);
    std::string sendGetRequestToRedirectedSite(char* host, char* path);
    
    std::unordered_map<std::string, std::string> location2proxy;
    std::string mime_type;
    int port_num;
    bool redirect = false;
}; 