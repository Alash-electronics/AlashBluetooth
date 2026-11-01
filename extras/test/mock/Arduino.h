// Имитация Arduino.h и Stream для тестов Bluetooth: время вручную.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <deque>
#include <string>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
inline uint32_t g_ms = 100000;
inline uint32_t millis() { return g_ms; }
inline void delay(uint32_t ms) { g_ms += ms; }
class Print {
 public:
  std::string log;
  virtual ~Print() {}
  virtual size_t write(const uint8_t *b, size_t n) { log.append((const char *)b, n); return n; }
  size_t print(const char *s) { return write((const uint8_t *)s, strlen(s)); }
  size_t print(const __FlashStringHelper *s) { return print((const char *)s); }
  size_t print(int v) { return print(std::to_string(v).c_str()); }
  size_t println(const char *s) { print(s); return print("\n"); }
  size_t println(int v) { print(v); return print("\n"); }
  size_t println(const __FlashStringHelper *s) { print(s); return print("\n"); }
};
class Stream : public Print {
 public:
  virtual int available() = 0;
  virtual int read() = 0;
};
