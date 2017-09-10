#pragma once

#include <ctime>
#include <string>

namespace cordpp {

  class Date {
  private:
    std::tm dtime;

  public:
    Date();
    Date(const std::time_t _time);
    Date(const std::string &datetime);

    static const std::time_t now();
    
    const int day() const;
    const int year() const;
    const int secs() const;
    const int mins() const;
    const int hours() const;
    const int month() const;
    const std::time_t getTime() const;
    const std::string toString() const;
    
    operator const char*();
    Date operator+(const Date& other);
    Date operator-(const Date& other);
    const bool operator> (const Date& d2);
    const bool operator<= (const Date& d2);
    const bool operator< (const Date& d2);
    const bool operator>= (const Date& d2);
  };

}