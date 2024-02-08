#include <crails/logger.hpp>
#include <sstream>

#undef NDEBUG
#include <cassert>

int main()
{
  using namespace std;
  using namespace Crails;

  // Log levels
  {
    stringstream output;

    logger.set_stdout(output);
    logger.set_stderr(output);
    logger << Logger::Info << "coucou ";
    logger << Logger::Warning << "petite ";
    logger << Logger::Error << "perruche";
    logger << Logger::endl;
    string result = output.str();
    assert(result.find("coucou") != string::npos);
    assert(result.find("petite") != string::npos);
    assert(result.find("perruche") != string::npos);
  }

  {
    stringstream output;

    logger.set_stdout(output);
    logger.set_stderr(output);
    logger.set_log_level(Logger::Warning);
    logger << Logger::Info << "coucou ";
    logger << Logger::Warning << "petite ";
    logger << Logger::Error << "perruche";
    logger << Logger::endl;
    string result = output.str();
    assert(result.find("coucou")   == string::npos);
    assert(result.find("petite")   != string::npos);
    assert(result.find("perruche") != string::npos);
  }

  // Support for splitting event and errors in separate streams
  {
    stringstream event, error;

    logger.set_stdout(event);
    logger.set_stderr(error);
    logger.set_log_level(Logger::Info);
    logger << Logger::Info << "coucou" << Logger::endl;
    logger << Logger::Warning << "petite" << Logger::endl;
    logger << Logger::Error << "perruche" << Logger::endl;
    logger << Logger::endl;
    string resultA = event.str();
    string resultB = error.str();
    assert(resultA.find("coucou")   != string::npos);
    assert(resultA.find("petite")   == string::npos);
    assert(resultA.find("perruche") == string::npos);
    assert(resultB.find("coucou")   == string::npos);
    assert(resultB.find("petite")   != string::npos);
    assert(resultB.find("perruche") != string::npos);
  }

  // Invokable logs
  {
    stringstream output;
    bool invoked = false;
    auto invokable = [&invoked]() -> string { invoked = true; return "hello"; };

    logger.set_stdout(output);
    logger.set_stderr(output);
    logger << Logger::Debug << invokable << Logger::endl;
    assert(invoked == false);
    logger << Logger::Error << invokable << Logger::endl;
    assert(output.str().find("hello") != string::npos);
    assert(invoked == true);
  }

  return 0;
}
