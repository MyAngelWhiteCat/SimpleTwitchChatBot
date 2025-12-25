#include "chat_bot.h"
#include "logging.h"

#include <utility>

namespace chat_bot {

    void ChatBot::SetCommandStart(char ch) {
        command_start_ = ch;
    }

    char ChatBot::GetCommandStart() const {
        return command_start_;
    }

    void ChatBot::AddCommand(std::string_view command_name, commands::Command&& command) {
        name_to_command_[std::string(command_name)] = std::move(command);
    }

    // case 1 - user:!command 
    // case 2 - user:!command some text for command execution
    void ChatBot::ParseAndExecute(irc::domain::Message&& message) {
        auto line = message.GetContent();
        if (line.empty()) {
            return;
        }

        std::string command;
        std::string content;

        if (line[0] == command_start_) {
            size_t command_end = line.find_first_of(' '); 
            if (command_end == std::string::npos) {
                command = std::string(line.substr(1));
            }
            else {
                command = std::string(line.substr(1, command_end - 1)); 
                content = std::string(line.substr(command_end + 1));
            }
        }
        else {
            LOG_INFO("Not a command");
            LOG_INFO(std::string(line));
            return;
        }

        if (auto it = name_to_command_.find(command); it != name_to_command_.end()) {
            it->second.AddContent(std::move(content));
            it->second.Execute(message.GetNick(), message.GetRole());
        }
        else {
            LOG_ERROR("Unknown command");
        }

    }
}
