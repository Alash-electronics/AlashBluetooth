# AlashBluetooth: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **Bluetooth-модулей HC-05, HC-06 и HM-10**: сама **определяет модуль и скорость**, меняет имя, PIN и скорость нужной AT-командой (у каждого модуля они разные, и у HC-06 команды вообще без Enter). Ещё принимает **текстовые команды от телефона** (`speed=80`). Объясняет, почему модуль молчит. Работает с любым `Stream`. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashBluetooth** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| Модуль | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX и RX соединяются **крест-накрест**: TX модуля к RX платы. RX модуля рассчитан на 3.3 В, к 5 В Uno его подключают через делитель. Режим AT: HC-05 удерживать кнопку при включении (светодиод мигает раз в 2 секунды, скорость 38400); HC-06 и HM-10 готовы к AT, пока не подключены к телефону.

### Быстрый старт
```cpp
#include <AlashBluetooth.h>
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);
void setBaud(long b) { BT.begin(b); }
AlashBluetooth bluetooth(BT, setBaud, 9600);

void setup() {
  Serial.begin(115200); BT.begin(9600);
  if (bluetooth.detect()) {              // тип и скорость сами / type and baud by itself
    bluetooth.setName("Phobo-01");
  } else {
    Serial.println(bluetooth.errorText());
  }
}
void loop() {}
```

### Что умеет
- **Находит модуль сама:** `detect()` пробует разные способы общения и перебирает скорости 9600, 38400, 115200 и другие; результат: HC-05, HC-06 или HM-10 и рабочая скорость.
- **Правильные команды для каждого модуля:** `setName()`, `setPin()`, `setBaudRate()`; для HC-06 без Enter и с паузой в секунду между командами, для HC-05 с Enter, для HM-10 свой набор.
- **Проверка значений:** имя до 12 (HM-10), 20 (HC-06) или 32 (HC-05) символов; PIN из 4 цифр (HC-06) или 6 цифр (HM-10); скорость из списка модуля.
- **Чтение:** `name()`, `version()`, `address()` (MAC), `reset()`.
- **Команды от телефона:** `readLine()` собирает строку без блокировки, `parseCommand("speed=80")` разбирает имя и число.
- **Понятные причины молчания:** провода крест-накрест, режим AT (у HC-05 кнопка при включении), телефон подключён, скорость.
- **Мост AT_Terminal:** пример для ручной отправки любых AT-команд.
- Работает на **Arduino AVR, ESP32, ESP8266** с любым `Stream`.

### Справочник по методам
| Метод | Что делает |
|---|---|
| `AlashBluetooth(stream, setBaud, baud)` | Модуль на потоке; функция смены скорости порта |
| `detect(scanBauds)` | Найти модуль и скорость |
| `setName(s) / setPin(s) / setBaudRate(b)` | Имя, PIN, скорость |
| `name() / version() / address() / reset()` | Чтение и перезапуск |
| `module() / moduleName() / baud()` | Тип и скорость |
| `readLine(buf, size)` | Собрать строку без блокировки |
| `parseCommand(line, name, size, &value)` | Разобрать "имя=число" |
| `errorText() / diagnose(Serial)` | Причина / самопроверка |

### Если что-то не работает
**Модуль молчит**

TX модуля к RX платы и RX модуля к TX платы. Для HC-05 держите кнопку при включении (мигает раз в 2 секунды). HC-06 и HM-10 не должны быть подключены к телефону. Запустите пример Detect.

**Имя не меняется на телефоне**

Телефон помнит старое имя. Удалите модуль из списка устройств Bluetooth на телефоне, выключите и включите модуль и найдите заново.

**После смены скорости модуль не отвечает**

Порт платы должен работать на новой скорости: библиотека переключает его через функцию setBaud. Если скорость задана неверно или запись не прошла, снова запустите `detect()`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Detect`, `Setup`, `AT_Terminal`, `Commands`

---

## Қазақша

**HC-05, HC-06 және HM-10 Bluetooth модульдеріне** арналған қарапайым кітапхана: **модуль мен жылдамдықты өзі анықтайды**, атын, PIN және жылдамдығын қажетті AT-командамен өзгертеді. Телефоннан **мәтін командаларын** (`speed=80`) қабылдайды. Модуль неге үндемейтінін түсіндіреді. Кез келген `Stream` арқылы жұмыс істейді. Тәуелділіксіз.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashBluetooth** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| Модуль | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX және RX **айқастыра** қосылады. AT режимі: HC-05 қосқанда батырманы ұстап тұрыңыз; HC-06 және HM-10 телефонға қосылмаған кезде AT-қа дайын.

### Жылдам бастау
```cpp
#include <AlashBluetooth.h>
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);
void setBaud(long b) { BT.begin(b); }
AlashBluetooth bluetooth(BT, setBaud, 9600);

void setup() {
  Serial.begin(115200); BT.begin(9600);
  if (bluetooth.detect()) {              // тип и скорость сами / type and baud by itself
    bluetooth.setName("Phobo-01");
  } else {
    Serial.println(bluetooth.errorText());
  }
}
void loop() {}
```

### Мүмкіндіктері
- **Модульді өзі табады:** `detect()` әр түрлі тәсілдерді көреді, жылдамдықтарды іріктейді.
- **Әр модульге дұрыс командалар:** `setName()`, `setPin()`, `setBaudRate()`.
- **Мәндерді тексеру:** аты, PIN, жылдамдық.
- **Оқу:** `name()`, `version()`, `address()`, `reset()`.
- **Телефоннан командалар:** `readLine()`, `parseCommand("speed=80")`.
- **Үндемеу себептері түсінікті:** сымдар, AT режимі, телефон, жылдамдық.
- **AT_Terminal көпірі.**
- **Arduino AVR, ESP32, ESP8266**.

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `AlashBluetooth(stream, setBaud, baud)` | Ағындағы модуль |
| `detect(scanBauds)` | Модуль мен жылдамдықты табу |
| `setName(s) / setPin(s) / setBaudRate(b)` | Аты, PIN, жылдамдық |
| `name() / version() / address() / reset()` | Оқу және қайта қосу |
| `module() / moduleName() / baud()` | Түрі мен жылдамдығы |
| `readLine(buf, size)` | Жолды бөгемей жинау |
| `parseCommand(line, name, size, &value)` | "аты=сан" талдау |
| `errorText() / diagnose(Serial)` | Себеп / өзін тексеру |

### Бірдеңе жұмыс істемесе
**Модуль үндемейді**

TX модульден тақта RX-іне, RX модульден тақта TX-іне. HC-05 үшін қосқанда батырманы ұстаңыз.

**Телефонда аты өзгермейді**

Телефон ескі атты есте сақтайды. Модульді тізімнен жойып, қайта іздеңіз.

**Жылдамдық өзгергеннен кейін модуль жауап бермейді**

Тақта порты жаңа жылдамдықта болуы керек. `detect()` қайта іске қосыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Detect`, `Setup`, `AT_Terminal`, `Commands`

---

## English

A simple library for **HC-05, HC-06 and HM-10 Bluetooth modules**: it **detects the module and the baud rate** by itself and changes the name, PIN and speed with the right AT command (they differ per module, and the HC-06 takes commands without Enter). It also receives **text commands from a phone** (`speed=80`) and explains why a module is silent. Works with any `Stream`. No dependencies. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashBluetooth** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| Module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX and RX are connected **crossed**: module TX to board RX. The module RX is 3.3 V, so on a 5 V Uno use a divider. AT mode: for the HC-05 hold the button at power-up (the LED blinks every 2 seconds, speed 38400); the HC-06 and HM-10 are ready for AT while no phone is connected.

### Quick start
```cpp
#include <AlashBluetooth.h>
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);
void setBaud(long b) { BT.begin(b); }
AlashBluetooth bluetooth(BT, setBaud, 9600);

void setup() {
  Serial.begin(115200); BT.begin(9600);
  if (bluetooth.detect()) {              // тип и скорость сами / type and baud by itself
    bluetooth.setName("Phobo-01");
  } else {
    Serial.println(bluetooth.errorText());
  }
}
void loop() {}
```

### Features
- **Finds the module itself:** `detect()` tries different ways of talking and scans the baud rates 9600, 38400, 115200 and more; the result is HC-05, HC-06 or HM-10 and the working speed.
- **The right commands per module:** `setName()`, `setPin()`, `setBaudRate()`; for the HC-06 without Enter and with a one-second gap between commands, for the HC-05 with Enter, for the HM-10 its own set.
- **Value checks:** name up to 12 (HM-10), 20 (HC-06) or 32 (HC-05) characters; PIN of 4 digits (HC-06) or 6 digits (HM-10); a baud rate from the module's list.
- **Reading:** `name()`, `version()`, `address()` (MAC), `reset()`.
- **Phone commands:** `readLine()` collects a line without blocking, `parseCommand("speed=80")` splits the name and the number.
- **Clear reasons for silence:** crossed wires, AT mode (the HC-05 needs its button at power-up), a connected phone, the baud rate.
- **AT_Terminal bridge:** an example for sending any AT command by hand.
- Works on **Arduino AVR, ESP32, ESP8266** with any `Stream`.

### Method reference
| Method | What it does |
|---|---|
| `AlashBluetooth(stream, setBaud, baud)` | Module on a stream; a function that changes the port speed |
| `detect(scanBauds)` | Find the module and baud rate |
| `setName(s) / setPin(s) / setBaudRate(b)` | Name, PIN, speed |
| `name() / version() / address() / reset()` | Read and reset |
| `module() / moduleName() / baud()` | Type and speed |
| `readLine(buf, size)` | Collect a line without blocking |
| `parseCommand(line, name, size, &value)` | Parse "name=number" |
| `errorText() / diagnose(Serial)` | Reason / self-check |

### If something does not work
**The module is silent**

Module TX to board RX and module RX to board TX. For the HC-05 hold the button at power-up (blinks every 2 seconds). The HC-06 and HM-10 must not be connected to a phone. Run the Detect example.

**The name does not change on the phone**

The phone remembers the old name. Remove the module from the phone's Bluetooth list, power-cycle it and search again.

**After changing the speed the module does not answer**

The board port must run at the new speed: the library switches it via the setBaud function. If the speed is wrong or the write failed, run `detect()` again.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Detect`, `Setup`, `AT_Terminal`, `Commands`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashBluetooth
