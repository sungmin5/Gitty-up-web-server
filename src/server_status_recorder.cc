
#include "server_status_recorder.h"

server_status_recorder& server_status_recorder::get_instance()
{
    static server_status_recorder instance_;
    return instance_;
}

void server_status_recorder::request_recorder(const std::string& url, const unsigned int response_code)
{   
    std::pair<std::string, unsigned int> url_response(url, response_code);
    
    std::lock_guard<std::mutex> lock(response_list_lock_);
    url_response_list_.push_back(url_response);
}

void server_status_recorder::prefix_recorder(const std::pair<std::string, std::string>& prefix_handler_pair)
{
    std::lock_guard<std::mutex> lock(prefix_list_lock_);
    prefix_handler_list_.push_back(prefix_handler_pair);
}

std::vector<std::pair<std::string, unsigned int>> server_status_recorder::get_url_response_list() const
{
    std::lock_guard<std::mutex> lock(response_list_lock_);
    return url_response_list_;
}

std::vector<std::pair<std::string, std::string>> server_status_recorder::get_prefix_handler_list() const
{
    std::lock_guard<std::mutex> lock(prefix_list_lock_);
    return prefix_handler_list_;
}

int server_status_recorder::get_url_response_list_size()
{
    std::lock_guard<std::mutex> lock(response_list_lock_);
    return url_response_list_.size();
}
