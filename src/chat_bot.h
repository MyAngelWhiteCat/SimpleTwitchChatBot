#pragma once 

#include "command.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>

#include <boost/asio.hpp>

namespace chat_bot {

    using Mode = commands::Command;

    namespace net = boost::asio;

    struct PrintableMessage {
        std::string name;
        std::string badge;
        std::string colour;
        std::string message;
    };

    class ChatBot : public std::enable_shared_from_this<ChatBot> {
    public:
        ChatBot(net::io_context& ioc)
            : ioc_(ioc)
        {

        }

        void ParseAndExecute(irc::domain::Message&& message);

        void SetCommandStart(char ch);
        char GetCommandStart() const;
        void AddCommand(std::string_view command_name, commands::Command&& command);
        void AddMode(Mode&& mode);

    private:
        net::io_context& ioc_;
        char command_start_ = '!';
        std::unordered_map<std::string, commands::Command> name_to_command_;
        std::vector<Mode> modes_;

        void UseMode(irc::domain::Message&& msg);
        void ProcessCommand(irc::domain::Message&& msg);
    };

}