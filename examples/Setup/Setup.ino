// AlashBluetooth: настройка модуля одной командой: имя в списке Bluetooth на телефоне, PIN и скорость.
// Не нужно помнить, что HC-06 принимает AT без Enter, а HC-05 с Enter, и чем отличаются команды: библиотека знает.
// Модуль должен быть в режиме AT (см. пример Detect) и не подключён к телефону.
#include <AlashBluetooth.h>
#if defined(ESP32)
#define BT Serial2                         // ESP32: аппаратный порт Serial2, RX = GPIO16, TX = GPIO17
void setBaud(long b) { Serial2.begin(b, SERIAL_8N1, 16, 17); }
#else
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);                 // Uno / Nano: RX = D10 (сюда TX модуля), TX = D11 (к RX модуля через делитель 1 кОм / 2 кОм)
void setBaud(long b) { BT.begin(b); }
#endif

AlashBluetooth bluetooth(BT, setBaud, 9600);

void setup() {
  Serial.begin(115200);
  setBaud(9600);
  if (!bluetooth.detect()) {               // сам находит модуль и скорость
    Serial.println(bluetooth.errorText());
    while (true) delay(1000);
  }
  Serial.print("Модуль: ");
  Serial.println(bluetooth.moduleName());

  if (bluetooth.setName("Phobo-01")) Serial.println("Имя установлено");
  else Serial.println(bluetooth.errorText());           // например, имя слишком длинное для этого модуля

  // PIN: у HC-06 ровно 4 цифры, у HM-10 ровно 6 цифр, у HC-05 любое число
  const char *pin = "1234";
  if (bluetooth.moduleName() && bluetooth.module() == AlashBluetooth::HM10) pin = "123456";
  if (bluetooth.setPin(pin)) Serial.println("PIN установлен");
  else Serial.println(bluetooth.errorText());

  // bluetooth.setBaudRate(9600);          // скорость обмена модуля (после смены порт платы переключается сам)
  Serial.println("Готово. Выключите и включите модуль: он появится в списке Bluetooth под новым именем.");
}

void loop() {}
