#ifndef  CRAILS_LOG_ROTATE_HPP
# define CRAILS_LOG_ROTATE_HPP

# include <fstream>
# include <string>

namespace Crails
{
  class LogRotate
  {
  public:
    virtual ~LogRotate();

    void initialize(const std::string& event_filepath, const std::string& error_filepath = "");
    void set_history_size(unsigned short);
    void swap_log_files();

  protected:
    void load_log_files();
    void log_rotate();

    std::ofstream event_log, error_log;
    std::string event_filepath, error_filepath;
    unsigned short history_size;
  };
}

#endif
