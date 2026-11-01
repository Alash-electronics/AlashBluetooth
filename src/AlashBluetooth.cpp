// MIT License. Alash Electronics.
#include "AlashBluetooth.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// Скорости для перебора: HC-06 и HM-10 по умолчанию 9600, HC-05 в режиме AT 38400
static const long BAUDS[] = {9600, 38400, 115200, 57600, 19200, 4800, 2400, 1200};

const __FlashStringHelper *AlashBluetooth::moduleName() const {
  return _mod == HC05 ? F("HC-05") : _mod == HC06 ? F("HC-06") : _mod == HM10 ? F("HM-10") : F("?");
}

void AlashBluetooth::flushInput() {
  while (_s->available()) _s->read();
}

// Отправляет команду и собирает ответ: ждём первый байт до waitMs, затем пока идут символы (пауза 150 мс = конец ответа).
// Модули HC-06 и HM-10 понимают команду без перевода строки по паузе в передаче, поэтому между такими командами держим секунду.
bool AlashBluetooth::query(const char *cmd, bool crlf, uint16_t waitMs) {
  static uint32_t lastSend = 0;
  if (!crlf) {
    while ((uint32_t)(millis() - lastSend) < 1100) delay(20);
  }
  flushInput();
  _s->print(cmd);
  if (crlf) _s->print("\r\n");
  lastSend = millis();
  size_t n = 0;
  uint32_t t0 = millis(), lastByte = 0;
  bool got = false;
  while (true) {
    while (_s->available()) {
      int c = _s->read();
      if (c < 0) break;
      got = true;
      lastByte = millis();
      if (c == '\r') continue;
      if (n < sizeof(_reply) - 1) _reply[n++] = (c == '\n') ? '\n' : (c >= 32 && c < 127 ? (char)c : '?');     // строки ответа разделены \n
    }
    uint32_t now = millis();
    if (got && (uint32_t)(now - lastByte) > 150) break;
    if (!got && (uint32_t)(now - t0) > waitMs) break;
    if (got && (uint32_t)(now - t0) > (uint32_t)waitMs + 1500) break;
    delay(5);
  }
  while (n > 0 && (_reply[n - 1] == ' ' || _reply[n - 1] == '\n')) n--;
  _reply[n] = 0;
  return got;
}

// Проверка связи на текущей скорости с определением типа
bool AlashBluetooth::tryAt() {
  // 1) HC-06 и HM-10 отвечают на "AT" без перевода строки
  if (query("AT", false, 500) && replyHas("OK")) {
    if (query("AT+VERR?", false, 600) && (replyHas("HMSoft") || replyHas("BT0") || replyHas("MLT") || replyHas("HM-") || replyHas("V"))) {
      // у HC-06 такой команды нет, он промолчит или ответит ошибкой; ответ со строкой версии это HM-10 (или его клон)
      if (!replyHas("ERROR") && !replyHas("linvor")) { _mod = HM10; return true; }
    }
    if (query("AT+VERSION", false, 600) && replyHas("OK")) { _mod = HC06; return true; }
    _mod = UNKNOWN;
    _err = ERR_UNKNOWN_MODULE;
    return true;
  }
  // 2) HC-05 требует перевод строки CR LF. Предыдущая проба оставила в нём "AT" без конца строки: достаточно досказать "\r\n"
  if (query("", true, 500) && replyHas("OK")) {
    _mod = HC05;
    return true;
  }
  return false;
}

bool AlashBluetooth::isAlive() {
  if (_mod == HC05) return query("AT", true, 500) && replyHas("OK");
  return query("AT", false, 500) && replyHas("OK");
}

bool AlashBluetooth::detect(bool scanBauds) {
  _mod = UNKNOWN;
  _err = ERR_OK;
  if (tryAt()) {
    if (_mod == UNKNOWN) return false;
    return true;
  }
  if (scanBauds && _setBaud) {
    for (uint8_t i = 0; i < sizeof(BAUDS) / sizeof(BAUDS[0]); i++) {
      _setBaud(BAUDS[i]);
      _baud = BAUDS[i];
      delay(50);
      if (tryAt()) {
        if (_mod == UNKNOWN) return false;
        return true;
      }
    }
  }
  _err = ERR_NO_REPLY;
  return false;
}

// ---- Настройка -------------------------------------------------------------------------------------

static bool printable(const char *s, size_t maxLen, bool allowSpace) {
  size_t n = strlen(s);
  if (n == 0 || n > maxLen) return false;
  for (size_t i = 0; i < n; i++) {
    unsigned char c = s[i];
    if (c < 32 || c > 126) return false;
    if (!allowSpace && c == ' ') return false;
  }
  return true;
}

bool AlashBluetooth::setName(const char *name) {
  if (_mod == UNKNOWN) { _err = ERR_UNKNOWN_MODULE; return false; }
  if (!printable(name, _mod == HM10 ? 12 : _mod == HC06 ? 20 : 32, _mod == HC05)) { _err = ERR_BAD_VALUE; return false; }
  char cmd[56];
  snprintf(cmd, sizeof cmd, _mod == HC05 ? "AT+NAME=%s" : "AT+NAME%s", name);
  bool ok = query(cmd, _mod == HC05, 800) && (replyHas("OK") || replyHas("Set"));
  _err = ok ? ERR_OK : (strlen(_reply) ? ERR_REFUSED : ERR_NO_REPLY);
  return ok;
}

bool AlashBluetooth::setPin(const char *pin) {
  if (_mod == UNKNOWN) { _err = ERR_UNKNOWN_MODULE; return false; }
  size_t n = strlen(pin);
  bool digits = n > 0;
  for (size_t i = 0; i < n; i++) if (pin[i] < '0' || pin[i] > '9') digits = false;
  if (!digits || (_mod == HM10 && n != 6) || (_mod == HC06 && n != 4) || (_mod == HC05 && n > 16)) { _err = ERR_BAD_VALUE; return false; }
  char cmd[40];
  snprintf(cmd, sizeof cmd, _mod == HC05 ? "AT+PSWD=%s" : _mod == HC06 ? "AT+PIN%s" : "AT+PASS%s", pin);
  bool ok = query(cmd, _mod == HC05, 800) && (replyHas("OK") || replyHas("Set"));
  if (!ok && _mod == HC05) {                                // прошивки HC-05 версии 3 и новее требуют пароль в кавычках
    snprintf(cmd, sizeof cmd, "AT+PSWD=\"%s\"", pin);
    ok = query(cmd, true, 800) && replyHas("OK");
  }
  _err = ok ? ERR_OK : (strlen(_reply) ? ERR_REFUSED : ERR_NO_REPLY);
  return ok;
}

bool AlashBluetooth::setBaudRate(long baud) {
  if (_mod == UNKNOWN) { _err = ERR_UNKNOWN_MODULE; return false; }
  char cmd[40];
  if (_mod == HC05) {
    snprintf(cmd, sizeof cmd, "AT+UART=%ld,0,0", baud);
  } else if (_mod == HC06) {
    // 1 = 1200, 2 = 2400, 3 = 4800, 4 = 9600, 5 = 19200, 6 = 38400, 7 = 57600, 8 = 115200
    static const long b[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    int code = -1;
    for (int i = 0; i < 8; i++) if (b[i] == baud) code = i + 1;
    if (code < 0) { _err = ERR_BAD_VALUE; return false; }
    snprintf(cmd, sizeof cmd, "AT+BAUD%d", code);
  } else {
    // HM-10: 0 = 9600, 1 = 19200, 2 = 38400, 3 = 57600, 4 = 115200, 5 = 4800, 6 = 2400, 7 = 1200
    static const long b[] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200};
    int code = -1;
    for (int i = 0; i < 8; i++) if (b[i] == baud) code = i;
    if (code < 0) { _err = ERR_BAD_VALUE; return false; }
    snprintf(cmd, sizeof cmd, "AT+BAUD%d", code);
  }
  bool ok = query(cmd, _mod == HC05, 800) && (replyHas("OK") || replyHas("Set"));
  _err = ok ? ERR_OK : (strlen(_reply) ? ERR_REFUSED : ERR_NO_REPLY);
  if (ok) {
    _baud = baud;
    if (_setBaud) _setBaud(baud);                           // порт платы переключаем на новую скорость
    delay(200);
  }
  return ok;
}

// Берёт из ответа часть после двоеточия или "OK" и кладёт в out
static void extract(const char *reply, char *out, size_t size) {
  const char *p = strchr(reply, ':');
  if (p) p++;
  else if (strncmp(reply, "OK", 2) == 0) p = reply + 2;
  else p = reply;
  while (*p == ' ') p++;
  size_t n = 0;
  while (p[n] && p[n] != '\n' && n + 1 < size) { out[n] = p[n]; n++; }    // только первая строка ответа (вторая это "OK")
  out[n] = 0;
}

bool AlashBluetooth::name(char *out, size_t size) {
  if (_mod == HC06) { _err = ERR_NOT_SUPPORTED; return false; }
  bool ok = query("AT+NAME?", _mod == HC05, 800) && (_mod == HC05 ? replyHas("+NAME") : replyHas("NAME") || replyHas("Get"));
  if (ok) extract(_reply, out, size);
  _err = ok ? ERR_OK : ERR_NO_REPLY;
  return ok;
}

bool AlashBluetooth::version(char *out, size_t size) {
  bool ok;
  if (_mod == HC05) ok = query("AT+VERSION?", true, 800) && replyHas("VERSION");
  else if (_mod == HM10) ok = query("AT+VERR?", false, 800) && strlen(_reply) > 0;
  else ok = query("AT+VERSION", false, 800) && replyHas("OK");
  if (ok) extract(_reply, out, size);
  _err = ok ? ERR_OK : ERR_NO_REPLY;
  return ok;
}

bool AlashBluetooth::address(char *out, size_t size) {
  if (_mod == HC06) { _err = ERR_NOT_SUPPORTED; return false; }
  bool ok = query("AT+ADDR?", _mod == HC05, 800) && replyHas("ADDR");
  if (ok) extract(_reply, out, size);
  _err = ok ? ERR_OK : ERR_NO_REPLY;
  return ok;
}

bool AlashBluetooth::reset() {
  if (_mod == HC06) { _err = ERR_NOT_SUPPORTED; return false; }
  bool ok = query("AT+RESET", _mod == HC05, 800) && (replyHas("OK") || replyHas("RESET"));
  _err = ok ? ERR_OK : ERR_NO_REPLY;
  return ok;
}

// ---- Текстовые команды ---------------------------------------------------------------------------

bool AlashBluetooth::readLine(char *buf, size_t size) {
  while (_s->available()) {
    int c = _s->read();
    if (c < 0) break;
    if (c == '\n' || c == '\r' || c == ';') {
      if (_lineLen == 0) continue;                      // пустая строка (второй символ CR LF)
      buf[_lineLen] = 0;
      _lineLen = 0;
      return true;
    }
    if (_lineLen + 1 < size) buf[_lineLen++] = (char)c;
  }
  return false;
}

bool AlashBluetooth::parseCommand(const char *line, char *name, size_t nameSize, long *value) {
  while (*line == ' ') line++;
  if (!*line) return false;
  size_t n = 0;
  while (*line && *line != '=' && *line != ':' && *line != ' ' && n + 1 < nameSize) name[n++] = *line++;
  name[n] = 0;
  while (*line == ' ' || *line == '=' || *line == ':') line++;
  if (value) *value = *line ? strtol(line, nullptr, 10) : 0;
  return n > 0;
}

// ---- Диагностика -------------------------------------------------------------------------------------

const __FlashStringHelper *AlashBluetooth::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_REPLY:
      return T("Модуль молчит. Проверьте: TX модуля к RX платы, RX модуля к TX платы (крест-накрест); питание; для HC-05 удерживайте кнопку на модуле при включении (мигает раз в 2 секунды); HC-06 и HM-10 не должны быть подключены к телефону. Скорость подбирается перебором",
               "Модуль үндемейді. Тексеріңіз: модуль TX-і тақта RX-іне, модуль RX-і тақта TX-іне (айқастыра); қуат; HC-05 үшін қосқанда модульдегі батырманы ұстап тұрыңыз; HC-06 және HM-10 телефонға қосылмауы керек",
               "The module is silent. Check: module TX to board RX and module RX to board TX (crossed); power; for the HC-05 hold the button while powering on (it blinks every 2 seconds); the HC-06 and HM-10 must not be connected to a phone");
    case ERR_UNKNOWN_MODULE:
      return T("Модуль отвечает, но тип не определился (клон с другой прошивкой)", "Модуль жауап береді, бірақ түрі анықталмады", "The module answers but its type could not be told (a clone with other firmware)");
    case ERR_REFUSED:
      return T("Модуль ответил отказом на команду. Посмотрите lastReply(). Возможно, он подключён к телефону или значение недопустимо",
               "Модуль команданы қабылдамады. lastReply() қараңыз. Телефонға қосылған болуы мүмкін",
               "The module refused the command. See lastReply(). It may be connected to a phone or the value is not allowed");
    case ERR_BAD_VALUE:
      return T("Недопустимое значение: имя до 12 (HM-10), 20 (HC-06) или 32 (HC-05) символов; PIN: 4 цифры (HC-06), 6 цифр (HM-10); скорость из списка модуля",
               "Жарамсыз мән: аты 12 (HM-10), 20 (HC-06) не 32 (HC-05) таңбаға дейін; PIN: 4 сан (HC-06), 6 сан (HM-10)",
               "Value not allowed: name up to 12 (HM-10), 20 (HC-06) or 32 (HC-05) characters; PIN: 4 digits (HC-06), 6 digits (HM-10); a baud rate from the module's list");
    case ERR_NOT_SUPPORTED:
      return T("Этот модуль не умеет такую команду (у HC-06 нет чтения имени, адреса и перезапуска)", "Бұл модуль мұндай команданы білмейді", "This module has no such command (the HC-06 cannot read its name or address or reset)");
  }
  return F("");
}

void AlashBluetooth::diagnose(Print &out) {
  out.println(T("AlashBluetooth: диагностика (перебираю скорости, это до минуты)", "AlashBluetooth: диагностика (жылдамдықтарды іріктеймін, бір минутқа дейін)", "AlashBluetooth: diagnostics (scanning baud rates, up to a minute)"));
  if (detect(true)) {
    out.print(T("Найден модуль ", "Модуль табылды ", "Module found: "));
    out.print(moduleName());
    out.print(T(", скорость ", ", жылдамдық ", ", baud "));
    out.println((int)_baud);
    char buf[40];
    if (version(buf, sizeof buf)) { out.print(T("Версия: ", "Нұсқа: ", "Version: ")); out.println(buf); }
    if (name(buf, sizeof buf)) { out.print(T("Имя: ", "Аты: ", "Name: ")); out.println(buf); }
    if (address(buf, sizeof buf)) { out.print(F("MAC: ")); out.println(buf); }
  } else {
    out.println(errorText());
  }
}
