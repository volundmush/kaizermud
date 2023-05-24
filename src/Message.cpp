#include "kaizermud/Message.h"

namespace kaizermud::game {

    boost::json::array Message::render() const {
        boost::json::array result;
        result.push_back(boost::json::string(msgType));
        result.push_back(args);
        result.push_back(kwargs);
        return result;
    }

}