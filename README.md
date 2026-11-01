# AlashBluetooth

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **Bluetooth-модулей HC-05, HC-06 и HM-10**: сама **определяет модуль и скорость**, меняет имя, PIN и скорость нужной AT-командой (у каждого модуля они разные, и у HC-06 команды вообще без Enter). Ещё принимает **текстовые команды от телефона** (`speed=80`). Объясняет, почему модуль молчит. Работает с любым `Stream`. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

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

### Подключение
| Модуль | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX и RX соединяются **крест-накрест**: TX модуля к RX платы. RX модуля рассчитан на 3.3 В, к 5 В Uno его подключают через делитель. Режим AT: HC-05 удерживать кнопку при включении (светодиод мигает раз в 2 секунды, скорость 38400); HC-06 и HM-10 готовы к AT, пока не подключены к телефону.

### Чем отличается от других библиотек
Мы посмотрели SoftwareSerial-примеры из курсов, HC-05 (Alexander Fedorov) и Bluetooth HC-06 (разные):
- Это AT-терминалы: вы сами вводите команды, а у HC-06 без Enter, у HC-05 с Enter, у HM-10 свои. Ученики теряются. У нас `setName()` для всех трёх.
- Не определяют скорость: нужно знать, что у HC-05 в режиме AT 38400. У нас перебор.
- Нет разбора команд от телефона; у нас `readLine()` и `parseCommand()`.

### Примеры
`Detect`, `Setup`, `AT_Terminal`, `Commands`

### Тесты
Определение HC-05 / HC-06 / HM-10 по форматам ответов, перебор скоростей (HC-05 на 38400, HM-10 на 115200, перепутанная скорость порта), настройка имени, PIN и скорости с проверкой значений, чтение версии, имени и MAC, повтор пароля в кавычках, паузы между командами (имитация теряет слитые команды), молчащий модуль, чтение строк по частям и разбор команд проверены без железа на имитации модулей: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**HC-05, HC-06 және HM-10 Bluetooth модульдеріне** арналған қарапайым кітапхана: **модуль мен жылдамдықты өзі анықтайды**, атын, PIN және жылдамдығын қажетті AT-командамен өзгертеді. Телефоннан **мәтін командаларын** (`speed=80`) қабылдайды. Модуль неге үндемейтінін түсіндіреді. Кез келген `Stream` арқылы жұмыс істейді. Тәуелділіксіз.

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

### Қосу
| Модуль | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX және RX **айқастыра** қосылады. AT режимі: HC-05 қосқанда батырманы ұстап тұрыңыз; HC-06 және HM-10 телефонға қосылмаған кезде AT-қа дайын.

### Басқа кітапханалардан айырмашылығы
Курс мысалдарын қарадық:
- Бұл AT-терминалдар: командаларды өзің енгізесің; HC-06-да Enter жоқ, HC-05-те бар. Бізде үшеуіне `setName()`.
- Жылдамдықты анықтамайды; бізде іріктеу.
- Телефоннан команданы талдау бізде бар.

### Мысалдар
`Detect`, `Setup`, `AT_Terminal`, `Commands`

### Тесттер
HC-05 / HC-06 / HM-10 анықтау, жылдамдықты іріктеу, баптау, оқу, командалар арасындағы үзіліс, үндемейтін модуль, жолдарды оқу тексерілген: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for **HC-05, HC-06 and HM-10 Bluetooth modules**: it **detects the module and the baud rate** by itself and changes the name, PIN and speed with the right AT command (they differ per module, and the HC-06 takes commands without Enter). It also receives **text commands from a phone** (`speed=80`) and explains why a module is silent. Works with any `Stream`. No dependencies. Made for classes and Alash Electronics kits.

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

### Wiring
| Module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В (HC-05, HC-06); 3.3 В (HM-10) | 3.3-5 В по модулю |
| GND | GND | GND |
| TX модуля | D10 (RX платы) | GPIO16 (RX2) |
| RX модуля | D11 (TX платы) через делитель 1 кОм / 2 кОм | GPIO17 (TX2) |

TX and RX are connected **crossed**: module TX to board RX. The module RX is 3.3 V, so on a 5 V Uno use a divider. AT mode: for the HC-05 hold the button at power-up (the LED blinks every 2 seconds, speed 38400); the HC-06 and HM-10 are ready for AT while no phone is connected.

### How it differs from other libraries
We looked at course SoftwareSerial examples and assorted HC-05 / HC-06 helpers:
- They are AT terminals: you type the commands, and the HC-06 takes no Enter, the HC-05 needs Enter and the HM-10 has its own. Students get lost. We have `setName()` for all three.
- They do not find the baud rate: you must know the HC-05 uses 38400 in AT mode. We scan.
- No parsing of phone commands; we have `readLine()` and `parseCommand()`.

### Examples
`Detect`, `Setup`, `AT_Terminal`, `Commands`

### Tests
Detecting HC-05 / HC-06 / HM-10 by their reply formats, baud scanning (HC-05 at 38400, HM-10 at 115200, a wrong port speed), setting name, PIN and speed with value checks, reading version, name and MAC, the quoted-password retry, the gaps between commands (the simulator drops merged commands), a silent module, reading lines in pieces and parsing commands are checked without hardware on module simulators: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
