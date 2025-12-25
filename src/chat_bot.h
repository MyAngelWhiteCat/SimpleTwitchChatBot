#pragma once 

#include "command.h"

#include <string>
#include <string_view>
#include <unordered_map>

namespace chat_bot {

    struct PrintableMessage {
        std::string name;
        std::string badge;
        std::string colour;
        std::string message;
    };

    class ChatBot {
    public:
        ChatBot()
        {

        }

        void ParseAndExecute(irc::domain::Message&& message);

        void SetCommandStart(char ch);
        char GetCommandStart() const;
        void AddCommand(std::string_view command_name, commands::Command&& command);

    private:
        char command_start_ = '!';
        std::unordered_map<std::string, commands::Command> name_to_command_;

    };

}