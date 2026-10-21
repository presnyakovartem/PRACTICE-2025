# DentoBase

DentoBase — приложение для хранения и обработки стоматологических снимков челюсти человека.

Текущая версия проекта содержит:

- PostgreSQL-схему БД;
- роли БД;
- функции и триггеры на стороне БД;
- Redis-кэширование;
- логирование через `spdlog`;
- конфигурирование через `.conf` файлы;
- генерацию 1000 записей в каждой таблице;
- полноценный Qt GUI;
- компонентную сборку через CMake.

Технологический консольный UI удалён. Актуальный пользовательский интерфейс — Qt GUI.

## Структура проекта

```text
DentoBase_current/
├── config/
│   ├── app.conf
│   ├── admin.conf
│   ├── doctor.conf
│   └── technician.conf
├── database/
│   ├── schema.sql
│   ├── functions.sql
│   ├── triggers.sql
│   ├── roles.sql
│   └── init/
├── scripts/
│   ├── reset.sh
│   ├── generate_seed.py
│   ├── apply_roles.sh
│   └── reset_database.sh
├── src/
│   ├── app/
│   ├── cache/
│   ├── config/
│   ├── domain/
│   ├── gui/
│   ├── logging/
│   ├── repositories/
│   ├── services/
│   └── sql_builders/
├── docker-compose.yml
└── CMakeLists.txt
```

## Зависимости

```bash
sudo apt update
sudo apt install cmake g++ pkg-config \
    libpqxx-dev libpq-dev \
    libspdlog-dev libfmt-dev \
    libhiredis-dev \
    qt6-base-dev
```

## Инфраструктура

PostgreSQL и Redis запускаются через Docker Compose:

```bash
docker-compose up -d postgres redis
```

или, если используется Docker Compose v2:

```bash
docker compose up -d postgres redis
```

## Полная пересборка базы данных

Для полного пересоздания БД, применения схемы, функций, триггеров, генерации тестовых данных и применения ролей используется единый скрипт:

```bash
./scripts/reset.sh
```

Скрипт выполняет:

1. запуск контейнеров PostgreSQL и Redis;
2. пересоздание базы `dentobase_test`;
3. применение `database/schema.sql`;
4. применение `database/functions.sql`;
5. применение `database/triggers.sql`;
6. генерацию `database/seed.sql` через `scripts/generate_seed.py`;
7. загрузку 1000 записей в каждую таблицу;
8. применение `database/roles.sql`;
9. вывод количества записей по таблицам.

## Сборка

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

После сборки появятся компоненты:

```text
build/dist/components/libdentobase_config.a
build/dist/components/libdentobase_logging.a
build/dist/components/libdentobase_cache.a
build/dist/components/libdentobase_data_access.a
build/dist/components/libdentobase_business.a
build/dist/components/libdentobase_gui.a
```

Исполняемый файл GUI:

```text
build/dist/dentobase_gui_app
```

## Запуск GUI

Перед запуском убедись, что контейнеры подняты и БД инициализирована:

```bash
./scripts/reset.sh
```

Запуск под ролью администратора:

```bash
./build/dist/dentobase_gui_app config/admin.conf
```

Запуск под ролью врача:

```bash
./build/dist/dentobase_gui_app config/doctor.conf
```

Запуск под ролью лаборанта:

```bash
./build/dist/dentobase_gui_app config/technician.conf
```

## Конфигурирование

Пример `config/admin.conf`:

```ini
db.connection_string=host=127.0.0.1 port=5432 dbname=dentobase_test user=dentobase_admin_user password=admin_password
log.file=logs/admin.log
log.level=debug
redis.enabled=true
redis.host=127.0.0.1
redis.port=6379
redis.ttl_seconds=300
business.max_diagnosis_text_length=2048
```

Параметры:

- `db.connection_string` — строка подключения к PostgreSQL;
- `log.file` — путь к файлу логов;
- `log.level` — `debug` или `release`;
- `redis.enabled` — включение Redis-кэширования;
- `redis.host` — адрес Redis;
- `redis.port` — порт Redis;
- `redis.ttl_seconds` — время жизни записи кэша;
- `business.max_diagnosis_text_length` — максимальная длина текста заключения.

## Логирование

Логирование реализовано через библиотеку `spdlog`.

Поддерживаются режимы:

- `debug` — пишутся все сообщения, включая `DEBUG`;
- `release` — пишутся сообщения `INFO`, `WARNING`, `ERROR`, но не `DEBUG`.

Файл логов задаётся в конфиге через `log.file`.

## Redis-кэширование

В проекте реализован компонент `dentobase_cache`, использующий Redis через `hiredis`.

Кэшируется поиск пациента:

```text
patient:<uuid>
```

При первом поиске пациента в логах появляется `Redis cache miss`, после загрузки из PostgreSQL пациент записывается в Redis. При повторном поиске появляется `Redis cache hit`.

## Роли БД

Реализованы роли:

| Роль | Пользователь | Назначение |
|---|---|---|
| `dentobase_admin` | `dentobase_admin_user` | полный доступ |
| `dentobase_doctor` | `dentobase_doctor_user` | просмотр медицинских данных и создание заключений |
| `dentobase_technician` | `dentobase_technician_user` | создание пациентов, медкарт и снимков |

Роли применяются скриптом `scripts/reset.sh` или отдельно:

```bash
./scripts/apply_roles.sh
```

## Генерация данных

Генератор:

```bash
python3 scripts/generate_seed.py
```

Создаёт файл:

```text
database/seed.sql
```

В каждую таблицу загружается не менее 1000 записей:

- `patients`;
- `clinics`;
- `doctors`;
- `doctor_clinics`;
- `technicians`;
- `equipment`;
- `image_types`;
- `medical_records`;
- `images`;
- `diagnoses`.

## GUI

GUI реализован на Qt 6 как отдельный компонент `dentobase_gui`.

GUI работает через тот же прикладной интерфейс `IApplicationService`, что и остальные компоненты приложения. Бизнес-логика и доступ к данным не дублируются в интерфейсе.

Поддерживаемые сценарии в GUI:

- создание и поиск пациента;
- создание и поиск медицинской карты;
- создание клиники;
- создание и поиск врача;
- создание лаборанта;
- создание оборудования;
- создание типа снимка;
- загрузка и поиск снимка;
- создание и поиск заключения;
- просмотр справочников.

