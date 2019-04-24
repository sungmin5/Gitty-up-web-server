# Gitty Up Web Server

This is an implementation of an HTTP 1.1 compliant web server written in c++.

## Source Code Layout

The following is a high-level overview of the repository:

* __cmake__ => CMake-specific files.
* __conf__ => Configuration files.
* __docker__ => Docker-specific files.
* __include__ => All header files.
* __resources__ => Static resources for the web server.
    * There is a subfolder for every static file serving route.
* __src__ => All implementation files.
* __tests__ => All unit and integration tests.
    * Certain tests have subfolders containing static files that they require.
* __CMakeLists.txt__ => CMake configuration file.

## Working with the Web Server

The following section will go over building, testing, and running the web server.

### Building

The steps to build the web server are as follows:

* Ensure all dependencies are installed on the target machine (see https://docs.google.com/document/d/1mL83U4DXll9oe-Vm4g3vbvLD6NoLxkfQ_J0zps6c_tY/edit#heading=h.lgmhii7a2jv1).
* Create a _build_ directory in the root of the repository. 
```mkdir build && cd build```
* Run _cmake_ from this directory pointing to _CMakeLists.txt_.
    * Note: If any new header files/dependencies/tests are added, _CMakeLists.txt_ must be updated to account for them.
```cmake ..```
* The above command generates a Makefile. The actual building can then be accomplished by executing the default _make_ task of this file.
``` make ```

### Testing

As part of the build process, tests are automatically targeted in _CMakeLists.txt_ such that a _make_ task is created specifically for testing. Once the server has been built, to run the existing tests, do the following:

* Execute the _test_ task of the generated Makefile (from the same build directory).
``` make test```

#### Adding New Tests

See below for instructions to add both new unit tests and new integration tests.

##### Unit Tests

To add a new unit test, create a new .cc file in the _tests_ directory. This project uses the Google.Test (https://github.com/google/googletest) unit testing suite. The provided link has sample usage.

Once the tests have been written, they must be added to the Makefile by updating _CMakeLists.txt_ appropriately. See https://docs.google.com/document/d/1mL83U4DXll9oe-Vm4g3vbvLD6NoLxkfQ_J0zps6c_tY/edit#heading=h.ctxvko37rx78 for details on how to do this.

##### Integration Tests

To add a new integration test, it is recommended that the script _tests/integration.sh_ is used. This script has the ability to make arbitrary cURL and nc requests to the web server via command line arguments. For specific documentation on the script's usage, run the following command:
``` tests/integration.sh -h```

Once the desired command has been written for the script, it must be added to the Makefile by updating _CMakeLists.txt_ appropriately. See https://docs.google.com/document/d/1mL83U4DXll9oe-Vm4g3vbvLD6NoLxkfQ_J0zps6c_tY/edit#heading=h.ctxvko37rx78 for details on how to do this. 


### Running

Once the server has been built, it can be run with the following commands:

* Update an existing configuration file or create a new configuration file for the server to use. See below for specific documentation on the configuration file.
* Navigate to the directory that the server should be run out of.
    * Unless otherwise specified in the configuration file, all paths in the configuration file will be relative to this directory.
* Run the _server_main_ executable.
    * Note: The web server takes a single command line argument which is the path to a configuration file.
``` [PATH TO BUILD DIR]/build/bin/server_main [PATH TO CONFIG FILE]```

### Configuring

All server-level parameters are specified in a configuration file. This file takes on an Nginx-like structure. Important parameters and details are provided below.

* __port [port]__ => The port on which the server should listen for incoming connections.
* __root [path]__ => The root path for the server. All other paths in the configuration file __should__ be relative to this root.
    * If the root is not specified or is set to ".", the directory from which the server is executed is set as the root.
* __router__ => A nested configuration for the router. Contains all route handler configurations.
    * __handler [name]__ => A nested configuration for the route handler with name _[name]_. Note that this name is used by the router to bind the appropriate route handler instance to any given request.
        * __location [location]__ => The URI for which the given handler should be invoked.
        * All other items in this nested configuration are subject to the specifics of the given handler.
        
Example:
```
port 8080;
root .;
router {
    handler echo {
        location /echo;
    }
    handler status {
        location /status;
    }
    handler static1 {
        location /static1;
        root resources/static1;
    }
    handler static2 {
        location /static2;
        root resources/static2;
    }
}
```

### Debugging

If the server is behaving in unexpected ways, it can be debugged using the generated log files. Log files are generated in the directory that the server is run from by default. All incoming requests and unhandled exceptions are written to the logs.

## Adding a New Request Handler

To add a new request handler, the developer must implement a derived instance of the following class:

```
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
};
```

Note that there are two public methods that must be implemented. These methods more than likely will delegate tasks to specific private methods that the developer will define as needed for his/her specific handler. See below for an example of a derived route handler:

```
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
```

Once the derived class has been implemented (requiring both a header file in _include_ and an implementation in _src_), it must be linked to the router by modifying _router.cc_ as follows:
* Include the new handler in _router.h_.
* Update the _select_handler_ method in _router.cc_ by adding a new clause to the if statement. See below for an example.
```
std::shared_ptr<route_handler> router::select_handler (std::string uri)
{
    std::string route = longest_prefix_match(uri);
    std::string handler = get_route_handler(route);
    
    if (handler == "echo")
    {
        return echo_handler::create_handler(get_handler_config("echo"), server_root_);
    }
    else if (handler == "static1")
    {
        return static_file_handler::create_handler(get_handler_config("static1"), server_root_);
    }
    ...
}
```

After this, _CMakeLists.txt_ must be updated to include the new handler as a dependency where necessary (see https://docs.google.com/document/d/1mL83U4DXll9oe-Vm4g3vbvLD6NoLxkfQ_J0zps6c_tY/edit#heading=h.ctxvko37rx78 for cmake details). Note that if unit tests and/or integration tests were written for this handler, additional updates to _CMakeLists.txt_ will be necessary.

Finally, the new handler must be registered to a URI(s). This can be accomplished by updating the configuration file for the web server as follows:
* Under the _router_ nested configuration, create a new nested configuration as __handler [name] { ... }__
    * __[name]__ should correspond exactly to the name in the new if clause of _select_handler_ in the router.
    * Every handler must have a _location_ parameter specifying the URI that it should be registered to.
    * The developer can add more parameters (in addition to _location_) as needed. In the example below, an additional _root_ parameter was added to specify the directory for the static files that this route serves.

Example:
```
handler static1 {
    location /static1;
    root resources/static1;
}
```

## Authors

* **Sung Min Anh**
* **Josh Kimmel**
* **Albert Li**
* **Sulmie Tran**