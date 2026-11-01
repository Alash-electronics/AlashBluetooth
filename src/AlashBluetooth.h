// AlashBluetooth: настройка Bluetooth-модулей HC-05, HC-06 и HM-10 (имя, пароль, скорость) без запоминания AT-команд,
// автоопределение модуля и скорости, приём текстовых команд от телефона. Без зависимостей. MIT License. Alash Electronics.
#ifndef ALASH_BLUETOOTH_H
#define ALASH_BLUETOOTH_H

#include <Arduino.h>

class AlashBluetooth {
 public:
  enum Module : uint8_t { UNKNOWN = 0, HC05, HC06, HM10 };
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_REPLY,         // модуль молчит: провода, скорость, режим AT
    ERR_UNKNOWN_MODULE,   // ответ есть, но тип модуля не определился
    ERR_REFUSED,          // модуль ответил ошибкой на команду
    ERR_BAD_VALUE,        // имя или пароль недопустимы для этого модуля
    ERR_NOT_SUPPORTED     // этот модуль не умеет такую команду
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  // stream: порт модуля (SoftwareSerial, Serial1, Serial2). setBaud: функция, которая меняет скорость порта (нужна для автоопределения скорости):
  //   void setBaud(long b) { bt.begin(b); }   (для SoftwareSerial / HardwareSerial)
  // currentBaud: скорость, на которой сейчас запущен порт (чтобы после detect() знать скорость, если модуль нашёлся сразу)
  explicit AlashBluetooth(Stream &stream, void (*setBaud)(long) = nullptr, long currentBaud = 0) : _s(&stream), _setBaud(setBaud), _baud(currentBaud) {}

  // ---- Определение -----------------------------------------------------------------
  bool detect(bool scanBauds = true);          // найти модуль: сначала на текущей скорости, потом перебором. true, если нашли
  bool isAlive();                              // модуль отвечает на "AT"
  Module module() const { return _mod; }
  const __FlashStringHelper *moduleName() const;
  long baud() const { return _baud; }          // скорость, на которой модуль отвечает (после detect)

  // ---- Настройка (модуль в режиме AT и не подключён к телефону) --------------------------
  bool setName(const char *name);              // имя, которое видно в списке Bluetooth на телефоне
  bool setPin(const char *pin);                // пароль/PIN для сопряжения, например "1234" (у HM-10 шесть цифр: "000000")
  bool setBaudRate(long baud);                 // скорость обмена модуля; после смены перенастройте порт
  bool name(char *out, size_t size);
  bool version(char *out, size_t size);
  bool address(char *out, size_t size);        // MAC-адрес (HC-05, HM-10)
  bool reset();                                // перезапуск модуля (HC-05, HM-10)
  const char *lastReply() const { return _reply; }

  // ---- Текстовые команды от телефона ------------------------------------------------------
  // readLine() собирает строку из пришедших символов без блокировки; возвращает true, когда строка закончилась (\n, \r или ';').
  bool readLine(char *buf, size_t size);
  static bool parseCommand(const char *line, char *name, size_t nameSize, long *value);   // "speed=80" -> name "speed", value 80; "go" -> name "go", value 0

  // ---- Диагностика -----------------------------------------------------------------
  Error error() const { return _err; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

 private:
  Stream *_s;
  void (*_setBaud)(long);
  long _baud;
  Module _mod = UNKNOWN;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  char _reply[48] = {0};
  uint8_t _lineLen = 0;
  bool query(const char *cmd, bool crlf, uint16_t waitMs = 600);
  bool replyHas(const char *s) const { return strstr(_reply, s) != nullptr; }
  void flushInput();
  bool tryAt();
};

#endif  // ALASH_BLUETOOTH_H
