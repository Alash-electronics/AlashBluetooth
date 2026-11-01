// Тесты AlashBluetooth без железа: имитация HC-05, HC-06 и HM-10 (форматы ответов, скорость порта, паузы между командами).
//   g++ -std=c++17 -Wall -I mock -I ../../src test_bluetooth.cpp ../../src/AlashBluetooth.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Module.h"
#include "AlashBluetooth.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)

int main() {
  // 1) определение типа на текущей скорости: HC-06, HM-10, HC-05
  FakeBt a; g_bt = &a; a.kind = FakeBt::HC06;
  AlashBluetooth h6(a, setHostBaud, 9600);
  CHECK(h6.detect() && h6.module() == AlashBluetooth::HC06 && h6.baud() == 9600 && a.dropped == 0);
  FakeBt b; g_bt = &b; b.kind = FakeBt::HM10; b.name = "HMSoft";
  AlashBluetooth h10(b, setHostBaud, 9600);
  CHECK(h10.detect() && h10.module() == AlashBluetooth::HM10 && b.dropped == 0);
  FakeBt c; g_bt = &c; c.kind = FakeBt::HC05; c.moduleBaud = c.hostBaud = 38400; c.name = "HC-05";
  AlashBluetooth h5(c, setHostBaud, 38400);
  CHECK(h5.detect() && h5.module() == AlashBluetooth::HC05);

  // 2) подбор скорости: HC-05 в режиме AT на 38400, порт платы на 9600; найдено, порт переключён
  FakeBt d; g_bt = &d; d.kind = FakeBt::HC05; d.moduleBaud = 38400; d.hostBaud = 9600;
  AlashBluetooth hd(d, setHostBaud, 9600);
  CHECK(hd.detect(true) && hd.module() == AlashBluetooth::HC05 && hd.baud() == 38400 && d.hostBaud == 38400);
  FakeBt e; g_bt = &e; e.kind = FakeBt::HM10; e.moduleBaud = 115200; e.hostBaud = 9600;
  AlashBluetooth he(e, setHostBaud, 9600);
  CHECK(he.detect(true) && he.module() == AlashBluetooth::HM10 && he.baud() == 115200);
  FakeBt e2; g_bt = &e2; e2.kind = FakeBt::HC06; e2.moduleBaud = 9600; e2.hostBaud = 38400;                // перепутана скорость порта
  AlashBluetooth he2(e2, setHostBaud, 38400);
  CHECK(!he2.detect(false) && he2.error() == AlashBluetooth::ERR_NO_REPLY);                                  // без перебора не нашёл
  CHECK(he2.detect(true) && he2.baud() == 9600 && he2.module() == AlashBluetooth::HC06);

  // 3) настройка HC-06: имя, PIN (ровно 4 цифры), скорость, версия; команды не сливаются (лишних потерь нет)
  FakeBt f; g_bt = &f; f.kind = FakeBt::HC06;
  AlashBluetooth w6(f, setHostBaud, 9600); w6.detect();
  CHECK(w6.setName("AlashCar") && f.name == "AlashCar");
  CHECK(w6.setPin("4321") && f.pin == "4321");
  CHECK(!w6.setPin("123") && w6.error() == AlashBluetooth::ERR_BAD_VALUE && !w6.setPin("12ab") && !w6.setPin("12345"));
  CHECK(!w6.setName("") && !w6.setName("a name with spaces") && !w6.setName("012345678901234567890") && w6.error() == AlashBluetooth::ERR_BAD_VALUE);
  char buf[48];
  CHECK(w6.version(buf, sizeof buf) && strstr(buf, "linvor") != nullptr);
  CHECK(!w6.name(buf, sizeof buf) && w6.error() == AlashBluetooth::ERR_NOT_SUPPORTED && !w6.address(buf, sizeof buf) && !w6.reset());
  CHECK(w6.setBaudRate(115200) && f.moduleBaud == 115200 && f.hostBaud == 115200 && w6.baud() == 115200);   // модуль и порт платы на новой скорости
  CHECK(!w6.setBaudRate(12345) && w6.error() == AlashBluetooth::ERR_BAD_VALUE);
  CHECK(w6.isAlive() && f.dropped == 0);

  // 4) настройка HM-10: имя до 12 символов, PIN ровно 6 цифр, скорость, MAC, версия, перезапуск
  FakeBt g; g_bt = &g; g.kind = FakeBt::HM10;
  AlashBluetooth w10(g, setHostBaud, 9600); w10.detect();
  CHECK(w10.setName("Phobo") && g.name == "Phobo");
  CHECK(!w10.setName("name_longer_than_12") && w10.error() == AlashBluetooth::ERR_BAD_VALUE);
  CHECK(w10.setPin("123456") && g.pin == "123456" && !w10.setPin("1234"));
  CHECK(w10.name(buf, sizeof buf) && strcmp(buf, "Phobo") == 0);
  CHECK(w10.address(buf, sizeof buf) && strcmp(buf, "001A7DDA7113") == 0);
  CHECK(w10.version(buf, sizeof buf) && strstr(buf, "HMSoft") != nullptr);
  CHECK(w10.reset());
  CHECK(w10.setBaudRate(115200) && g.moduleBaud == 115200);
  CHECK(g.dropped == 0);

  // 5) настройка HC-05: имя, пароль (в том числе с кавычками для новой прошивки), скорость, MAC, версия
  FakeBt h; g_bt = &h; h.kind = FakeBt::HC05; h.moduleBaud = h.hostBaud = 38400;
  AlashBluetooth w5(h, setHostBaud, 38400); w5.detect();
  CHECK(w5.setName("Robot Phobo") && h.name == "Robot Phobo");                                              // у HC-05 пробелы разрешены
  CHECK(w5.setPin("1234") && h.pin == "1234");
  h.quotedRequired = true; CHECK(w5.setPin("5678") && h.pin == "5678");                                      // прошивка просит кавычки: повтор сам
  CHECK(w5.name(buf, sizeof buf) && strcmp(buf, "Robot Phobo") == 0);                                          // из ответа "+NAME:Robot Phobo" и "OK" взята только первая строка
  CHECK(w5.version(buf, sizeof buf) && strstr(buf, "2.0-20100601") != nullptr);
  CHECK(w5.address(buf, sizeof buf) && strstr(buf, "98d3") != nullptr);
  CHECK(w5.setBaudRate(9600) && h.moduleBaud == 9600 && h.hostBaud == 9600);
  CHECK(w5.reset());

  // 6) модуль молчит (не в режиме AT, подключён к телефону): ERR_NO_REPLY, настройка невозможна
  FakeBt i; g_bt = &i; i.kind = FakeBt::HC06; i.connectedToPhone = true;
  AlashBluetooth wi(i, setHostBaud, 9600);
  uint32_t t0 = g_ms; CHECK(!wi.detect(true) && wi.error() == AlashBluetooth::ERR_NO_REPLY && g_ms - t0 > 10000);
  CHECK(!wi.setName("x") && wi.error() == AlashBluetooth::ERR_UNKNOWN_MODULE);

  // 7) чтение строк: \n, \r\n, ;, по частям, переполнение буфера
  FakeBt s; AlashBluetooth ln(s);
  char line[16];
  s.tx = "speed=80\r\nstop;go"; CHECK(ln.readLine(line, sizeof line) && strcmp(line, "speed=80") == 0);
  CHECK(ln.readLine(line, sizeof line) && strcmp(line, "stop") == 0);
  CHECK(!ln.readLine(line, sizeof line));                                                                    // "go" ещё не закончилась
  s.tx = "!\n"; CHECK(ln.readLine(line, sizeof line) && strcmp(line, "go!") == 0);
  s.tx = "0123456789abcdefghijkl\n"; CHECK(ln.readLine(line, sizeof line) && strlen(line) == 15);          // длинная обрезается, буфер цел
  s.tx = "\r\n\r\n"; CHECK(!ln.readLine(line, sizeof line));

  // 8) разбор команды "имя=значение"
  char name[16]; long v = -1;
  CHECK(AlashBluetooth::parseCommand("speed=80", name, sizeof name, &v) && strcmp(name, "speed") == 0 && v == 80);
  CHECK(AlashBluetooth::parseCommand("  led: -5", name, sizeof name, &v) && strcmp(name, "led") == 0 && v == -5);
  CHECK(AlashBluetooth::parseCommand("go", name, sizeof name, &v) && strcmp(name, "go") == 0 && v == 0);
  CHECK(!AlashBluetooth::parseCommand("   ", name, sizeof name, &v));
  CHECK(AlashBluetooth::parseCommand("verylongcommandname=1", name, 6, &v) && strlen(name) == 5);

  // 9) тексты и diagnose
  FakeBt j; g_bt = &j; j.kind = FakeBt::HM10; AlashBluetooth dg(j, setHostBaud, 9600);
  dg.setLanguage(AlashBluetooth::LANG_KK); const char *kk = (const char *)dg.errorText();
  dg.setLanguage(AlashBluetooth::LANG_EN); Print out; dg.diagnose(out);
  CHECK(kk != nullptr && out.log.find("HM-10") != std::string::npos && out.log.find("HMSoft") != std::string::npos);

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
