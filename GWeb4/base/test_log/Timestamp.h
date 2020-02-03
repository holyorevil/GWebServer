#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

/*时间戳函数，我的理解是创建一个microSecondsSinceEpoch_为0的结构体，
 *然后每次通过这个结构体创建子的microSecondsSinceEpoch_为当前时间的结构体，并且子结构体都在microSecondsSinceEpoch_为0的结构体
 *中进行运算操作*/
#include <Types.h>

#include <boost/operators.hpp>

namespace muduo
{

///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : public boost::less_than_comparable<Timestamp>//继承这个类，对于<,>,<=,>=这些运算符号，只需要定义
                  //<号，其他的都可以自动帮你定义了
{
 public:
  ///
  /// Constucts an invalid Timestamp.
  ///
  Timestamp()
    : microSecondsSinceEpoch_(0)
  {
  }

  ///
  /// Constucts a Timestamp at specific time
  ///
  /// @param microSecondsSinceEpoch
  explicit Timestamp(int64_t microSecondsSinceEpoch);

  void swap(Timestamp& that)//将两个Timestamp类中的microSecondsSinceEpoch_变量交换
  {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  // default copy/assignment/dtor are Okay

  char* toString() const;
  char* toFormattedString() const;

  bool valid() const { return microSecondsSinceEpoch_ > 0; }//判断microSecondsSinceEpoch_是否有效，大于0就有效

  // for internal usage.
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }//返回microSecondsSinceEpoch_
  time_t secondsSinceEpoch() const//返回以秒为单位的microSecondsSinceEpoch_
  { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

  ///
  /// Get time of now.
  ///
  static Timestamp now();//创建一个当前的时间的Timestamp结构体
  static Timestamp invalid();//创建一个microSecondsSinceEpoch_=0的Timestamp结构体,都是静态函数，可以直接调用

  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  int64_t microSecondsSinceEpoch_;//以微秒为单位
};

/*放在类外重载，但是普通数据类型可以使用原来的，遇到特定类型才会使用这个*/
inline bool operator<(Timestamp lhs, Timestamp rhs)//只需要定义<号，其他都自动定义，less_than_comparable模板作用
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microseciond
/// resolution for next 100 years.

//计算两个Timestamp之间的差，以秒为单位
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
//加时间
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}
#endif  // MUDUO_BASE_TIMESTAMP_H
