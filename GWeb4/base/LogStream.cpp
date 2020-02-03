#include "LogStream.h"
#include <algorithm>

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

// From muduo
template <typename T>
size_t convert(char buf[], T value) //将数值转换成字符串
{
  T i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

template <typename T>
void LogStream::formatInteger(T v) {
  // buffer容不下kMaxNumericSize个字符的话会被直接丢弃
  if (buffer.avail() >= kMaxNumericSize) {
    size_t len = convert(buffer.current(), v);
    buffer.add(len);
  }
}

LogStream& LogStream::operator<<(bool v) {
    buffer.append(v ? "1" : "0", 1);
    return *this;
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer.avail() >= kMaxNumericSize) 
  {
    int len = snprintf(buffer.current(), kMaxNumericSize, "%.12g", v);//%.12g表示数据长度最多为12位，并且如果更多，采用指数形式输出
    buffer.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(long double v) {
  if (buffer.avail() >= kMaxNumericSize) 
  {
    int len = snprintf(buffer.current(), kMaxNumericSize, "%.12Lg", v);
    buffer.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(char v) {
    buffer.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char* str) {
    if (str)
    buffer.append(str, strlen(str));
    else
    buffer.append("(null)", 6);
    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string& v) {
    buffer.append(v.c_str(), v.size());
    return *this;
}
