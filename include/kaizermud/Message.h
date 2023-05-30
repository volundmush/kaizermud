#pragma once
#include "kaizermud/base.h"
#include <list>
#include <boost/json.hpp>


namespace kaizer {
    struct Message {
        std::string cmd;
        boost::json::array args;
        boost::json::object kwargs;
    };
}