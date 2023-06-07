#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "kaizermud/startup.h"
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    // Define the command-line options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("i", po::value<std::string>()->default_value("127.0.0.1:7000"), "the IP:port for the local link to Thermite")
            ("l", po::value<int>()->default_value(5), "log level");

    // Parse the command-line arguments
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Print help message
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    // Get the IP:port and log level values
    std::string ip_port = vm["i"].as<std::string>();
    int log_level = vm["l"].as<int>();

    // Create an address object without using an io_context
    boost::asio::ip::tcp::endpoint endpoint;
    try {
        std::size_t colon = ip_port.find(':');
        if (colon == std::string::npos) {
            throw std::runtime_error("Invalid IP:port format");
        }

        std::string ip = ip_port.substr(0, colon);
        unsigned short port = std::stoi(ip_port.substr(colon + 1));

        boost::asio::ip::address address = boost::asio::ip::make_address(ip);
        endpoint = boost::asio::ip::tcp::endpoint(address, port);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    kaizer::registerBaseResources();
    kaizer::startup(endpoint);

    return 0;
}