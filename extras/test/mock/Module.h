// Имитация модулей HC-05 / HC-06 / HM-10 на уровне байтов: скорость порта, режим AT, паузы между командами, ответы настоящих форматов.
#pragma once
#include "Arduino.h"
struct FakeBt : public Stream {
  enum Kind { HC05, HC06, HM10 } kind = HC06;
  long moduleBaud = 9600, hostBaud = 9600;
  bool atMode = true, connectedToPhone = false;
  std::string name = "HC-06", pin = "1234", rx, tx; uint32_t lastWrite = 0, lastReply = 0, readyAt = 0; bool pending = false;
  int commands = 0, dropped = 0;
  size_t write(const uint8_t *b, size_t n) override { rx.append((const char *)b, n); lastWrite = g_ms; pending = true; return n; }
  int available() override { process(); return (int)tx.size(); }
  int read() override { process(); if (tx.empty()) return -1; char c = tx[0]; tx.erase(0, 1); return (unsigned char)c; }
  void say(const std::string &s) { if (hostBaud == moduleBaud) tx += s; else tx += std::string(s.size(), '\xff'); }   // чужая скорость: мусор
  void process() {
    if (!pending || rx.empty()) return;
    bool crlf = rx.size() >= 2 && rx.substr(rx.size() - 2) == "\r\n";
    bool idle = g_ms - lastWrite >= 100;
    if (kind == HC05 ? !crlf : !idle) return;                                 // HC-05 ждёт перевод строки, остальные паузу после команды
    std::string cmd = rx; rx.clear(); pending = false;
    if (crlf) cmd = cmd.substr(0, cmd.size() - 2);
    if (!atMode || connectedToPhone) return;                                  // вне режима AT или при связи с телефоном модуль молчит
    if (kind != HC05 && lastReply && g_ms - lastReply < 800) { dropped++; return; }   // HC-06 и HM-10 не успевают: слитые команды теряются
    if (hostBaud != moduleBaud) { tx += std::string(3, '\xff'); return; }
    commands++; lastReply = g_ms;
    handle(cmd);
  }
  void handle(const std::string &c) {
    auto starts = [&](const char *p) { return c.rfind(p, 0) == 0; };
    if (kind == HC06) {
      if (c == "AT") say("OK");
      else if (c == "AT+VERSION") say("OKlinvorV1.8");
      else if (starts("AT+NAME")) { name = c.substr(7); say("OKsetname"); }
      else if (starts("AT+PIN")) { pin = c.substr(6); say("OKsetPIN"); }
      else if (starts("AT+BAUD")) { static const long b[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}; int k = c[7] - '0'; if (k >= 1 && k <= 8) { say("OK" + std::to_string(b[k])); moduleBaud = b[k]; hostBaudAfter = true; } else say("ERROR"); }
    } else if (kind == HM10) {
      if (c == "AT") say("OK");
      else if (c == "AT+VERR?") say("HMSoft V540");
      else if (c == "AT+NAME?") say("OK+NAME:" + name);
      else if (starts("AT+NAME")) { name = c.substr(7); say("OK+Set:" + name); }
      else if (c == "AT+ADDR?") say("OK+ADDR:001A7DDA7113");
      else if (starts("AT+PASS")) { pin = c.substr(7); say("OK+Set:" + pin); }
      else if (starts("AT+BAUD")) { static const long b[] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400}; int k = c[7] - '0'; if (k >= 0 && k <= 8) { say("OK+Set:" + std::to_string(k)); moduleBaud = b[k]; } else say("ERROR"); }
      else if (c == "AT+RESET") say("OK+RESET");
    } else {
      if (c == "AT") say("OK\r\n");
      else if (c == "AT+VERSION?") say("+VERSION:2.0-20100601\r\nOK\r\n");
      else if (c == "AT+NAME?") say("+NAME:" + name + "\r\nOK\r\n");
      else if (starts("AT+NAME=")) { name = c.substr(8); say("OK\r\n"); }
      else if (c == "AT+ADDR?") say("+ADDR:98d3:31:fd1234\r\nOK\r\n");
      else if (starts("AT+PSWD=\"")) { pin = c.substr(9, c.size() - 10); say("OK\r\n"); }
      else if (starts("AT+PSWD=")) { quotedRequired ? say("FAIL\r\n") : (pin = c.substr(8), say("OK\r\n")); }
      else if (starts("AT+UART=")) { long nb = atol(c.c_str() + 8); say("OK\r\n"); moduleBaud = nb; }
      else if (c == "AT+RESET") say("OK\r\n");
      else say("ERROR:(0)\r\n");
    }
  }
  bool quotedRequired = false, hostBaudAfter = false;
};
inline FakeBt* g_bt = nullptr;
inline void setHostBaud(long b) { if (g_bt) g_bt->hostBaud = b; }
