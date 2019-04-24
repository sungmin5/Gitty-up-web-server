//A handler for HTTP requests that don't match with any other handlers

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "default_handler.h"

//overriden constructor takes a config
//for this route, no config is used so the input is expected to be NULL
default_handler::default_handler (std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) {}

//overriden factory method to create a new instance
std::shared_ptr<route_handler> default_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
    return std::shared_ptr<default_handler>(new default_handler(config, root_path));
}

//overridden method in parent class to handle a request
std::shared_ptr<response> default_handler::handle_request (std::shared_ptr<request> req)
{
    return generate_404_response(req);
}

//the provided URI did not match any defined handler
//so send a 404 response with an appropriate message body
std::shared_ptr<response> default_handler::generate_404_response (std::shared_ptr<request> req)
{
    std::shared_ptr<response> resp(new response(404, ("The requested resource at " + req->get_uri() + " could not be found!")));
    resp->set_header("Content-Type", "text/plain");
    return resp;
}