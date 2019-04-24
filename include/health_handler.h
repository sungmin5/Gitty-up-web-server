// Health handler that always returns 200 
#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>
#include "route_handler.h"

class health_handler : public route_handler
{
public:
  // Methods
  static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
  std::shared_ptr<response> handle_request (std::shared_ptr<request> req); // generate response
private:
  // Methods
  health_handler (std::shared_ptr<NginxConfig> config, std::string root_path); // constructor overload
  std::shared_ptr<response> generate_200_response (std::shared_ptr<request> req); // create 200 response
};

#endif
