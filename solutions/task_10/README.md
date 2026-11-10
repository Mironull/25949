# Задание 10: код завершения команды

Программа `run_command` запускает команду из первого аргумента, передаёт ей
остальные аргументы, ждёт завершения и печатает код завершения. Если команда
убита сигналом, программа печатает номер и описание сигнала.

```sh
make          # собрать run_command и тестовые программы crash_program и exit_code
make test     # прогнать все проверки
```

## Проверки

| Команда | Ожидаемый результат |
|---|---|
| `./run_command ls -l /tmp` | код 0 |
| `./run_command grep nonexistent_string_123 /etc/passwd` | код 1 |
| `./run_command fake_command_123` | `execvp: No such file or directory`, код 127 |
| `./run_command ./crash_program` | сигнал 11 (Segmentation fault) |
| `./run_command ./exit_code 42` | код 42 |
