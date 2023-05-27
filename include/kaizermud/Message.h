#pragma once
#include "kaizermud/base.h"
#include <list>
#include <boost/json.hpp>


namespace kaizermud::game {
    struct Message {
        std::string msgType;
        boost::json::array args;
        boost::json::object kwargs;
        boost::json::object options;
        boost::json::array render() const;
    };
}