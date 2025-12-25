#include "irc_client.h"
#include "chat_bot.h"

#include <memory>
#include <thread>
#include <vector>
#include <utility>

#include <boost/asio/io_context.hpp>


namespace net = boost::asio;

template <typename Fn>
void RunWorkers(unsigned int num_workers, Fn&& fn) {
    std::vector<std::jthread> works;
    works.reserve(num_workers);

    for (unsigned i = 0; i < num_workers; ++i) {
        works.emplace_back(fn);
    }
    fn();
}

int main() {
    unsigned thread_pool = 2;
    net::io_context ioc(thread_pool);
    auto wg = net::make_work_guard(ioc);

    auto chat_bot = std::make_shared<chat_bot::ChatBot>();

    auto test_executor = std::make_unique<commands::TestOutputCommandExecutor>();
    commands::Command command(std::move(test_executor));
    chat_bot->AddCommand("test", std::move(command));

    auto client = std::make_shared<irc::Client<chat_bot::ChatBot>>(ioc, chat_bot);

    irc::domain::AuthorizeData auth_data;
    client->Connect();
    client->Authorize(auth_data);
    client->CapRequest();
    client->Join("myangelwhitecat");
    client->Read();


    RunWorkers(thread_pool, [&ioc]() {
        ioc.run();
        });
}


