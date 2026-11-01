// AlashBluetooth: «мост» для ручных AT-команд. Всё, что вы пишете в мониторе порта (скорость 115200), уходит в модуль,
// а ответ модуля показывается в мониторе. Удобно пробовать любые команды из документации модуля.
// Важно: в мониторе порта выберите «Нет конца строки» для HC-06 и HM-10 и «Новая строка и возврат каретки» (CR+LF) для HC-05.
#include <AlashBluetooth.h>
#if defined(ESP32)
#define BT Serial2                         // ESP32: аппаратный порт Serial2, RX = GPIO16, TX = GPIO17
void setBaud(long b) { Serial2.begin(b, SERIAL_8N1, 16, 17); }
#else
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);                 // Uno / Nano: RX = D10 (сюда TX модуля), TX = D11 (к RX модуля через делитель 1 кОм / 2 кОм)
void setBaud(long b) { BT.begin(b); }
#endif

void setup() {
  Serial.begin(115200);
  setBaud(9600);                           // для HC-05 в режиме AT замените на 38400
  Serial.println("Пишите AT-команды. Модуль должен быть в режиме AT.");
}

void loop() {
  if (BT.available()) Serial.write(BT.read());
  if (Serial.available()) BT.write(Serial.read());
}
