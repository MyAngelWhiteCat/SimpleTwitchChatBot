#pragma once

#include "user_validator.h"

#include <string_view>


namespace commands {

    class BaseCommandExecutor {
    public:
        virtual void operator()([[maybe_unused]] std::string_view content) = 0;
    
    };

    class TestOutputCommandExecutor : public BaseCommandExecutor {
    public:

        void operator()([[maybe_unused]] std::string_view content) override {
            std::cout << content << "\n";
        }

    };

}

