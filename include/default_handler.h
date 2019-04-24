//A handler for HTTP requests that don't match with any other handlers
#ifndef DEFAULT_HANDLER_H
#define DEFAULT_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "route_handler.h"

class default_handler : public route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
private:    
    //Methods
    default_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::shared_ptr<response> generate_404_response (std::shared_ptr<request> req); //create a 404 response
};

#endif

