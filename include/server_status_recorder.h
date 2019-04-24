//where it records request uri and response.
#ifndef SERVER_STATUS_RECORDER_H
#define SERVER_STATUS_RECORDER_H

#include <string>
#include <vector>
#include <mutex>

class server_status_recorder
{
public:
    //singleton instance
    static server_status_recorder& get_instance();
    
    //get request url and response code
    void request_recorder(const std::string& url, const unsigned int response_code);
    //get prefix and handler pair
    void prefix_recorder(const std::pair<std::string, std::string>& prefix_handler_pair);

    //return list of request url and response code pair
    std::vector<std::pair<std::string, unsigned int>> get_url_response_list() const;
    //return list of prefix and handler pair
    std::vector<std::pair<std::string, std::string>> get_prefix_handler_list() const;

    //return size of url and code list
    int get_url_response_list_size();

private:
    //private constructor since singleton 
    server_status_recorder() {};

    //list of request url and response code
    std::vector<std::pair<std::string, unsigned int>> url_response_list_;
    std::vector<std::pair<std::string, std::string>> prefix_handler_list_;
    
    mutable std::mutex response_list_lock_;
    mutable std::mutex prefix_list_lock_;
};

#endif
