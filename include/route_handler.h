//An interface to be implemented by all route handlers designed to handle specific requests
#ifndef ROUTE_HANDLER_H
#define ROUTE_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include "config_parser.h"
#include "request.h"
#include "response.h"

class route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    virtual std::shared_ptr<response> handle_request (std::shared_ptr<request> req)=0; //given a request, generate an appropriate response
protected:    
    //Attributes
    route_handler (std::shared_ptr<NginxConfig> config, std::string root_path) { config_ = config; root_path_ = root_path; }; //constructor overload
    std::shared_ptr<NginxConfig> config_;
    std::string root_path_;
    
    mutable std::mutex config_lock_;
};

#endif

