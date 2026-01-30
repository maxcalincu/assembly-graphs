## library/core

[code overview](./docs/code.md)

[tools overview](./docs/tools.md)

## Требования
Чтобы успешно запустить проект требуется:
- ОС: Debian, Ubuntu или совместимый Linux-дистрибутив
- Компилятор: GCC/Clang для C++20 и выше
- Зависимости: Библиотека **Boost** (требуется компонент ``program_options``)


Если вы не хотите настраивать локальное окружение, воспользуйтесь готовым виртуальным окружением в [**Github Codebase**](https://docs.github.com/en/codespaces/developing-in-a-codespace/creating-a-codespace-for-a-repository)

Установка зависимостей:

```bash
sudo apt update && sudo apt install libboost-all-dev
```

## Как собрать проект и запустить все тесты

Из корневой директории запускаем:

```bash
./build.sh && cd build
make -j8 test_core && ./test_core
```

## Запускаем произвольный тул

Из **build** директории запускаем:

```bash
make -j8 ${tool_name}
./${tool_name} --help
```

## Доступные тулзы
- construct_set
- search_tw