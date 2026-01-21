## library/core

[code overview](./docs/code.md)

[scripts overview](./docs/scripts.md)

### Как собрать и запустить тесты library/core

Из корневой директории запускаем:

```bash
./build.sh && cd build
make -j8 test_core
./test_core
```

Теперь запускаем произвольный скрипт

```bash
make -j8 script_name
./script_name --help
```