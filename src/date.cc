#include "date.h"
#include <iomanip>

using namespace cordpp;

Date::Date() : Date(now()) {
}

Date::Date(const std::time_t _time) {
  dtime = *(std::gmtime(&_time));
}

Date::Date(const std::string &datetime) {
  static const char* format = "%Y-%m-%dT%H:%M:%SZ";
  std::istringstream ss{ datetime.c_str() };
  ss >> std::get_time(&dtime, format);
}

const std::time_t Date::now() {
  return std::time(nullptr);
}

const int Date::day() const {
  return dtime.tm_mday;
}

const int Date::year() const {
  return dtime.tm_year + 1900;
}

const int Date::month() const {
  return dtime.tm_mon + 1;
}

const int Date::secs() const {
  return dtime.tm_sec;
}

const int Date::mins() const {
  return dtime.tm_min;
}

const int Date::hours() const {
  return dtime.tm_hour;
}

const std::time_t Date::getTime() const {
  return std::mktime((std::tm*)(&dtime));
}

const std::string Date::toString() const {
  char t[126] = {0};
  static const char* format = "%Y-%m-%dT%H:%M:%SZ";
  std::strftime(t, sizeof(t), format, &dtime);
  return std::string(t);
}

Date Date::operator+(const Date& other) {
  return Date(this->getTime() + other.getTime());
}

Date Date::operator-(const Date& other) {
  return Date(this->getTime() - other.getTime());
}

const bool Date::operator> (const Date& d2) {
  return this->getTime() > d2.getTime();
}

const bool Date::operator<= (const Date& d2) {
  return this->getTime() <= d2.getTime();
}

const bool Date::operator< (const Date& d2) {
  return this->getTime() < d2.getTime();
}

const bool Date::operator>= (const Date& d2) {
  return this->getTime() >= d2.getTime();
}

Date::operator const char*() {
  return this->toString().c_str();
}

std::ostream& operator<< (std::ostream &stream, const Date &date) {
  stream << date.toString();
  return stream;
}