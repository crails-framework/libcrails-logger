#include <crails/log_rotate.hpp>
#include <crails/logger.hpp>
#include <filesystem>
#include <fstream>
#include <functional>

#undef NDEBUG
#include <cassert>
  
using namespace std;
using namespace Crails;

static const filesystem::path testdir("/tmp/logrotate");

static string read_file(const filesystem::path& path)
{
  ifstream file(path.string(), std::ios::binary);
  string out;
  size_t size, begin, end;

  begin = file.tellg();
  file.seekg(0, std::ios::end);
  end   = file.tellg();
  file.seekg(0, std::ios::beg);
  size  = end - begin;
  out.resize(size);
  file.read(&out[0], size);
  file.close();
  return out;
}

static void logrotate_test(std::function<void()> test)
{
  filesystem::create_directories(testdir);
  filesystem::current_path(testdir);
  test();
  filesystem::remove_all(testdir);
}

int main()
{
  // Output logs into a single file
  logrotate_test([&]()
  {
    LogRotate logfiles;
    filesystem::path logpath = testdir / "event.log";
    string log;

    logfiles.initialize(logpath.string());
    assert(filesystem::exists(logpath));
    logger << Logger::Info << "vraiment c'est vrai ?" << Logger::endl;
    logger << Logger::Error << "ca a vraiment existe ?" << Logger::endl;
    log = read_file(logpath);
    assert(log.find("vraiment c'est vrai ?") != string::npos);
    assert(log.find("ca a vraiment existe ?") != string::npos);
  });

  // Can display error logs in a separate file
  logrotate_test([&]()
  {
    LogRotate logfiles;
    filesystem::path eventpath = testdir / "event.log";
    filesystem::path errorpath = testdir / "error.log";
    string eventlog, errorlog;

    logfiles.initialize(eventpath.string(), errorpath.string());
    assert(filesystem::exists(eventpath));
    assert(filesystem::exists(errorpath));
    logger << Logger::Info << "vraiment c'est vrai ?" << Logger::endl;
    logger << Logger::Error << "ca a vraiment existe ?" << Logger::endl;
    eventlog = read_file(eventpath);
    errorlog = read_file(errorpath);
    assert(eventlog.find("vraiment c'est vrai ?") != string::npos);
    assert(errorlog.find("vraiment c'est vrai ?") == string::npos);
    assert(eventlog.find("ca a vraiment existe ?") == string::npos);
    assert(errorlog.find("ca a vraiment existe ?") != string::npos);
  });

  // Can rotate logs
  logrotate_test([&]()
  {
    filesystem::path logpath = testdir / "event.log";
    string log0, log1, log1_backup;

    {
      LogRotate logfiles;
      logfiles.initialize(logpath.string());
      logger << Logger::Info << "vous pouvez m'assurer" << Logger::endl;
    }

    log1_backup = read_file(logpath);

    {
      LogRotate logfiles;
      logfiles.initialize(logpath.string());
      logger << Logger::Info << "que tout cela a vraiment existe" << Logger::endl;
    }

    log0 = read_file(logpath);
    log1 = read_file(logpath.string() + ".0");
    assert(log1 == log1_backup);
    assert(log0.find("que tout cela") != string::npos);
    assert(log1.find("que tout cela") == string::npos);
    assert(log0.find("vous pouvez m") == string::npos);
    assert(log1.find("vous pouvez m") != string::npos);

    {
      LogRotate logfiles;
      logfiles.set_history_size(0);
      logfiles.initialize(logpath.string());
      logger << Logger::Info << "j'ai besoin de toucher et devoir pour savoir" << Logger::endl;
    }

    assert(filesystem::exists(logpath.string() + ".0"));
    assert(!filesystem::exists(logpath.string() + ".1"));
  });
  return 0;
}
