#include "log_rotate.hpp"
#include "logger.hpp"
#include <vector>
#include <filesystem>
#include <iostream>

using namespace std;
using namespace Crails;

LogRotate::~LogRotate()
{
  if (event_log.is_open())
    event_log.close();
  if (error_log.is_open())
    error_log.close();
}

void LogRotate::initialize(const string& event_filepath_, const string& error_filepath_)
{
  event_log.exceptions(std::ios::failbit | std::ios::badbit);
  event_filepath = event_filepath_;
  error_filepath = error_filepath_;
  if (event_filepath.length() > 0 && filesystem::exists(event_filepath)
   || error_filepath.length() > 0 && filesystem::exists(error_filepath))
    swap_log_files();
  else
  {
    lock_guard<mutex> guard(logger.mutex);
    load_log_files();
  }
}

static filesystem::path log_filepath(const string& filepath, unsigned short i)
{
  return filesystem::path(filepath + '.' + to_string(i));
}

void LogRotate::set_history_size(unsigned short size)
{
  history_size = size;
  for (const string& filepath : vector<string>{event_filepath, error_filepath})
  {
    unsigned short i = size + 1;

    while (filesystem::exists(log_filepath(filepath, i)))
      filesystem::remove(log_filepath(filepath, i++));
  }
}

void LogRotate::swap_log_files()
{
  unsigned short i = 1;
  string reference_path = event_filepath.length() > 0 ? event_filepath : error_filepath;

  while (filesystem::exists(log_filepath(reference_path, i)) && i <= history_size)
    ++i;
  if (i >= history_size)
  {
    log_rotate();
    i = history_size;
  }
  {
    lock_guard<mutex> guard(logger.mutex);

    if (event_log.is_open())
    {
      event_log.close();
      event_log.clear();
    }
    if (error_log.is_open())
    {
      error_log.close();
      error_log.clear();
    }
    if (event_filepath.length() > 0)
      filesystem::rename(event_filepath, log_filepath(event_filepath, i));
    if (error_filepath.length() > 0)
      filesystem::rename(error_filepath, log_filepath(error_filepath, i));
    load_log_files();
  }
}

void LogRotate::log_rotate()
{
  for (unsigned short i = 2 ; i <= history_size ; ++i)
  {
    if (event_filepath.length() > 0 && filesystem::exists(log_filepath(event_filepath, i)))
      filesystem::rename(log_filepath(event_filepath, i), log_filepath(event_filepath, i - 1));
    if (error_filepath.length() > 0 && filesystem::exists(log_filepath(error_filepath, i)))
      filesystem::rename(log_filepath(error_filepath, i), log_filepath(error_filepath, i - 1));
  }
}

void LogRotate::load_log_files()
{
  if (event_filepath.length() != 0)
  {
    event_log.open(event_filepath.c_str());
    if (event_log.is_open())
    {
      logger << Logger::Debug << "logs redirected to `" << event_filepath << '`' << Logger::endl;
      logger.stdout = &event_log;
      if (error_filepath.length() == 0)
        logger.stderr = &event_log;
    }
    else
      cerr << "failed to open log file `" << event_filepath << '`' << endl;
  }
  if (error_filepath.length() != 0)
  {
    logger << Logger::Debug << "error logs redirected to `" << error_filepath << '`' << Logger::endl;
    error_log.open(error_filepath.c_str());
    if (error_log.is_open())
      logger.stderr = &error_log;
    else
      cerr << "failed to open log file `" << error_filepath << '`' << endl;
  }
}
