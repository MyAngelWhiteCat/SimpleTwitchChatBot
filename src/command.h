#pragma once

#include <string>
#include <utility>
#include <memory>

#include "message.h"
#include "command_executor.h"
#include "user_validator.h"

namespace commands {

    class Command {
    public:
        Command() = default;

        Command(std::unique_ptr<BaseCommandExecutor>&& executor)
            : executor_(std::move(executor))
        {

        }

        void Execute(std::string_view user_name, irc::domain::Role user_role);

        void AddContent(std::string&& content);
        void AddContent(std::string_view content);

        void SetMinimumUserRole(irc::domain::Role role);

        void SetWhiteListOnly(bool status);
        void AddUserInWhiteList(std::string_view user_name);
        void RemoveUserFromWhiteList(std::string_view user_name);
        void AddUserInBlackList(std::string_view user_name);
        void RemoveUserFromBlackList(std::string_view user_name);
        void SetRoleLevel(int level);
        int GetRoleLevel();
        bool GetWhiteListOnly() const;
        std::unordered_set<std::string>* GetWhiteList();
        std::unordered_set<std::string>* GetBlackList();

    private:
        std::unique_ptr<BaseCommandExecutor> executor_{nullptr};
        user_validator::UserVerificator verificator_;

        irc::domain::Role minimum_user_role_{3};
        std::string content_;
    };

}