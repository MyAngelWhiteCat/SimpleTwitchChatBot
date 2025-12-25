
# Простая C++ библиотека для интеграции с чатом Twitch

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
