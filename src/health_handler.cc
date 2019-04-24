
#include "health_handler.h"

// overriden construcotr takes a config
// for  this route, no config is used so the input is expected to be NULL
health_handler::health_handler (std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) {}

// overriden factory method to create a new instance
std::shared_ptr<route_handler> health_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
  return std::shared_ptr<health_handler>(new health_handler(config, root_path));
}

//overridden method in parent class to handle a request
std::shared_ptr<response> health_handler::handle_request (std::shared_ptr<request> req)
{
  if(req->get_uri() == "/health")
  {
    return generate_200_response(req);
  }
  else
  {
    std::shared_ptr<response> resp(new response(400, "Requested file was not found!\n"));
    resp->set_header("Content-Type", "text/plain");
    return resp;
  }
}

//send a 200 response with an appropriate message body
std::shared_ptr<response> health_handler::generate_200_response (std::shared_ptr<request> req)
{
  std::shared_ptr<response> resp(new response(200, "OK"));
  resp->set_header("Content-Type", "text/plain");
  return resp;
}
