//A handler for HTTP requests that echoes the body of the request in the response

#include "echo_handler.h"

//overriden constructor takes a config
//for this route, no config is used so the input is expected to be NULL
echo_handler::echo_handler (std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) {}

//overriden factory method to create a new instance
std::shared_ptr<route_handler> echo_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
    return std::shared_ptr<echo_handler>(new echo_handler(config, root_path));
}

//overridden method in parent class to handle a request
std::shared_ptr<response> echo_handler::handle_request (std::shared_ptr<request> req)
{
    return generate_echo_response(req);
}

//echo the HTTP request received in the body of the response
//since validation occurs earlier, always respond with a 200
std::shared_ptr<response> echo_handler::generate_echo_response (std::shared_ptr<request> req)
{
    std::string header_check = req->get_header("Thread-Sleep");
    if (header_check != "")
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    std::shared_ptr<response> resp(new response(200, std::string(req->get_raw_request())));
    resp->set_header("Content-Type", "text/plain");
    return resp;
}