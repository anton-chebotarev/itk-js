#include <emscripten/bind.h>

#include <iostream>
#include <iomanip>

#include <libde265/de265.h>

namespace
  {
  void Test()
    {       
    std::cout << "de265_get_version: " << de265_get_version() << std::endl;
    }
  }

using namespace emscripten;
EMSCRIPTEN_BINDINGS(Test)
  {
  function("Test", &Test);
  }
