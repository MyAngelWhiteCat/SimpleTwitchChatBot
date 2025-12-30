#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <string>
#include <string_view>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "chat_bot.h"
#include "connection.h"
#include "message.h"

namespace irc {

    namespace handler {

        using namespace std::literals;

        namespace net = boost::asio;
        namespace sys = boost::system;
        using Strand = net::strand<net::io_context::executor_type>;
        using MessageType = irc::domain::MessageType;

        class MessageHandler : public std::enable_shared_from_this<MessageHandler> {

        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, Strand& connection_strand)
                : connection_(connection)
                , connection_strand_(connection_strand)
            {

            }

            void operator()(std::vector<domain::Message>&& messages);

            void UpdateConnection(std::shared_ptr<connection::Connection> new_connection);
            void SetChatBot(std::shared_ptr<chat_bot::ChatBot> chat_bot);

        private:
            Strand& connection_strand_;
            std::shared_ptr<connection::Connection> connection_;
            std::shared_ptr<chat_bot::ChatBot> chat_bot_{ nullptr };

            void SendPong(const std::string_view ball);
        };

        void MessageHandler::operator()(std::vector<domain::Message>&& messages) {
            try {
                std::stringstream ss{};
                for (auto& message : messages) {
                    switch (message.GetMessageType()) {
                    case MessageType::PING:
                        SendPong(message.GetContent());
                        break;
                    case MessageType::PRIVMSG:
                        ss << '[' << static_cast<int>(message.GetRole()) << ']' << message.GetNick() << ' ' << message.GetContent() << "\n";
                        LOG_INFO(ss.str());
                        if (!chat_bot_) {
                            LOG_INFO("Chat bot not setted");
                            return;
                        }
                        net::post([self = this->shared_from_this(), message = std::move(message)]() mutable
                            {
                                self->chat_bot_->ParseAndExecute(std::move(message));
                            });
                        break;
                    }
                }

            }
            catch (const std::exception& e) {
                LOG_CRITICAL("Handling "s.append(e.what()));
            }
        }

        void MessageHandler::UpdateConnection(std::shared_ptr<connection::Connection> new_connection) {
            net::dispatch(connection_strand_, [self = this->shared_from_this(), new_connection]() {
                self->connection_ = new_connection;
                });
        }

        void MessageHandler::SetChatBot(std::shared_ptr<chat_bot::ChatBot> chat_bot) {
            chat_bot_ = chat_bot;
        }

        void MessageHandler::SendPong(const std::string_view ball) {
            net::dispatch(connection_strand_, [self = this->shared_from_this(), ball = std::string(ball)]() {
                self->connection_->AsyncWrite(std::string(domain::Command::PONG).append(ball).append("\r\n"));
                });
        }

    }

}
