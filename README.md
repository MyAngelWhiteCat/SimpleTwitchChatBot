
# Простая C++ библиотека для интеграции с чатом Twitch

Ядром системы является класс Client, который реализует подключение к серверам twitch чата. В данном классе присутсвуют методы для чтения чата, а так же взаимодействия с самим irc: 
CapRequest, обработка Ping-Pong, Join/Part каналов. Поддерживает чтение нескольких каналов сразу.

Класс Message в паре с MessageProcessor представляет собой простой набор инструментов для работы с сырым irc сообщением. Определяет его тип, к примеру PRIVMSG (сообщение от пользователя)
или ROOMSTATE (состояние чата - участники, фоллоу мод, только смайлики) а так же прочие типы. Его основными инструментами являются методы для получения никнейма, роли (вип, модератор, саб) а так же контента - самого сообщения пользователя. Чат бот владеет этой информацией, в связи с чем способен гибко подстраиваться под каждого пользователя.

Валидатор пользователя - обязатльный атрибут команды. Он определяет какую минимальную роль должен иметь пользователь, чтобы использовать команду. Так же для каждой команды пользователя можно разместить в белый и черный список.

Данная реализация не позволяет вносить изменения в рантайме. Но эту опцию легко добавить, а настройки можно сериализовать. Примером может послужить мой проект [OsuRequestFlow](https://github.com/MyAngelWhiteCat/OsuRequestFlow). На основе данной библиотеки я реализовал систему автоматической загрузки карт для ритм игры osu!, ссылку на которую зритель отправляет в чат, чтобы стример ее сыграл. В нем как раз реализзована возможность изменения настроек в рантайме, а так же их сериализация и сохраение в JSON формате.

Все методы кроме подключения - ассинхронные. 

Клиент довольно гибкий, и сам по себе уже является неплохой библиотекой для взаимождействия с twitch irc

## Добавление пользовательской команды

Чтобы создать новую команду, выполните следующие шаги:

1. **Создайте класс-исполнитель команды**, унаследовав его от `BaseCommandExecutor` и переопределив `operator()`.

```cpp
class BaseCommandExecutor {
public:
    virtual void operator()([[maybe_unused]] std::string_view content) = 0;
};

// Пример исполнителя команды
class TestOutputCommandExecutor : public BaseCommandExecutor {
public:
    void operator()([[maybe_unused]] std::string_view content) override {
        std::cout << content << "\n";
    }
};
```

2. **Создайте экземпляр чат бота и добавьте команду** в основном приложении:

```cpp
auto chat_bot = std::make_shared<chat_bot::ChatBot>();

auto test_executor = std::make_unique<commands::TestOutputCommandExecutor>();
commands::Command command(std::move(test_executor));

chat_bot->AddCommand("test", std::move(command));
```

Кроме команды можно добавить мод:

```
chat_bot->AddMode(std::move(command));
```
Мод отличается от команды тем, что применяется к каждому сообщению, не требуя специального символа для запуска.
К примеру [OsuRequestFlow](https://github.com/MyAngelWhiteCat/OsuRequestFlow) реализует мод, который ищет в каждом сообщении ссылку на
карту ритм игры osu! и сразу ее скачивает.

## Пример использования

```cpp
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
    std::vector<std::jthread> workers;
    workers.reserve(num_workers);

    for (unsigned i = 0; i < num_workers; ++i) {
        workers.emplace_back(fn);
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
```

## TODO

- [ ] Добавить авторизацию (сейчас доступно только анонимное чтение чата, без отправки сообщений)
- [ ] Отправка сообщений в чат. Добавить именно сам метод в irc::Client (Невозможно без авторизации)

## Требования

- **Conan 2.x** (менеджер зависимостей)
- **CMake** (система сборки)

### Начальная настройка (если Conan не установлен)

1. Установите Conan:
```bash
pip install conan
conan profile detect --force
```

2. Добавьте Conan в PATH (если директория скриптов pip ещё не добавлена).

3. Соберите проект:
```bash
mkdir build && cd build
conan install .. --build=missing --output-folder=. -s build_type=Release -s compiler.runtime=static
# Примечание: Первая сборка может занять больше часа (компиляция библиотек Boost)
cmake .. --preset conan-default
cmake --build . --config Release
# Внимание: Может появиться много предупреждений компилятора (это нормально)
```
