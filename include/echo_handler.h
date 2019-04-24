//A handler for HTTP requests that echoes the body of the request in the response
#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <chrono>
#include "route_handler.h"

class echo_handler : public route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
private:    
    //Methods
    echo_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::shared_ptr<response> generate_echo_response (std::shared_ptr<request> req); //create an echo response for the given request
};

#endif

