#ifndef  CRAILS_LOGGER_HPP
# define CRAILS_LOGGER_HPP

# include <sstream>
# include <thread>
# include <mutex>

namespace Crails
{
  class LogRotate;

  class Logger
  {
    friend class LogRotate;

    template<typename T, bool callable = std::is_invocable<T>::value>
    struct Appender
    {
      static void append(std::ostream& stream, const T& value) { stream << value; }
    };

    template<typename T>
    struct Appender<T, true>
    {
      static void append(std::ostream& stream, const T& value) { stream << value(); }
    };
  public:
    enum Symbol
    {
      Debug = 0,
      Info,
      Warning,
      Error,
      endl
    };

    struct Buffer
    {
      Buffer();

      std::stringstream stream;
      Symbol            level;
    };

    Logger();

    void set_stdout(std::ostream& stream);
    void set_stderr(std::ostream& stream);

    Logger& operator<<(Symbol level);

    template<typename T>
    Logger& operator<<(const T item)
    {
      if (log_level <= buffer.level)
        Appender<T>::append(buffer.stream, item);
      return *this;
    }

    void flush();

    unsigned char get_log_level() const { return log_level; }
    void set_log_level(Symbol value) { log_level = value; }
  private:
    static Symbol log_level;
    static thread_local Buffer buffer;
    std::mutex    mutex;
    std::ostream* stdout_;
    std::ostream* stderr_;
  };

  extern Logger logger;
}

#endif
