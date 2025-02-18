#include <iostream>
#include <iomanip>
#include <atomic>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#ifdef _WIN32
# include <Windows.h>
typedef DWORD pid_t;
#else
# include <unistd.h>
#endif
#include "logger.hpp"

using namespace std;
using namespace Crails;

struct LoggerId
{
  unsigned long thread_id;
  pid_t         pid;

  LoggerId()
  {
    static atomic<unsigned long> value(0);

    thread_id = ++value;
#ifdef _WIN32
    pid = GetCurrentProcessId();
#else
    pid = getpid();
#endif
  }
};

static thread_local const LoggerId logger_id;

#define log_prefix << '[' << logger_id.pid << 'T' << (logger_id.thread_id) << "] "

namespace Crails
{
  Logger logger;
  thread_local Logger::Buffer Logger::buffer;
  Logger::Symbol Logger::log_level = Logger::Debug;
}

Logger::Buffer::Buffer() : level(Logger::endl)
{
}

Logger::Logger()
{
  stdout_ = &cout;
  stderr_ = &cerr;
}

void Logger::set_stderr(ostream& stream)
{
  mutex.lock();
  stderr_ = &stream;
  mutex.unlock();
}

void Logger::set_stdout(ostream& stream)
{
  mutex.lock();
  stdout_ = &stream;
  mutex.unlock();
}

void Logger::flush()
{
  using namespace boost::posix_time;
  struct tm    t = to_tm(second_clock::local_time());
  stringstream time_stream;

  time_stream << setfill('0');
  time_stream << '[' << setw(2) << t.tm_mday << '/' << setw(2) << (t.tm_mon + 1) << ' ';
  time_stream << setw(2) << t.tm_hour << ':' << setw(2) << t.tm_min << ':' << setw(2) << t.tm_sec << "] ";
  buffer.stream.seekg(0, ios::end);
  if (buffer.stream.tellg() > 0 && mutex.try_lock())
  {
    switch (buffer.level)
    {
      case Info:
        *stdout_ log_prefix << time_stream.str() << buffer.stream.str();
        stdout_->flush();
        break ;
      default:
        *stderr_ log_prefix << time_stream.str() << buffer.stream.str();
        stderr_->flush();
        break ;
    }
    mutex.unlock();
    buffer.stream.str("");
    buffer.stream.clear();
  }
}

Logger& Logger::operator<<(Symbol level)
{
  if (level == Logger::endl)
    *this << "\n\r";
  if (buffer.level != level)
    flush();
  if (level != Logger::endl)
    buffer.level = level;
  return *this;
}

