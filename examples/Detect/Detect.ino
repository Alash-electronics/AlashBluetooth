// AlashBluetooth: какой у меня модуль и на какой скорости? Скетч переберёт скорости и определит HC-05, HC-06 или HM-10,
// покажет имя, версию и адрес. Модуль должен быть в режиме AT:
//   HC-05: удерживайте кнопку на модуле при включении (светодиод мигает раз в 2 секунды), скорость 38400;
//   HC-06 и HM-10: просто включите, не подключайтесь с телефона (светодиод мигает).
// Подключение: TX модуля к RX платы, RX модуля к TX платы (крест-накрест), VCC 5 В (HM-10: 3.3 В), GND.
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
  bluetooth.setLanguage(AlashBluetooth::LANG_RU);   // LANG_KK: қазақша, LANG_EN: English
  bluetooth.diagnose(Serial);              // перебирает скорости (до минуты) и печатает всё, что нашёл
}

void loop() {}
