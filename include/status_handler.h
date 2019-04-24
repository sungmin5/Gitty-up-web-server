//A handler for that shows the status of the web server
#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "route_handler.h"
#include "server_status_recorder.h"

class status_handler : public route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
private:    
    //Methods
    status_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::shared_ptr<response> generate_status_response (std::shared_ptr<request> req, std::string body); //create a status response
};

#endif
