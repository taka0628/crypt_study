#include <boost/python.hpp>
#include "calc.h"

std::string hello()
{
    return "hello world";
}

int add_cpp(int x, int y)
{
    return x + y;
}

BOOST_PYTHON_MODULE(CModule)
{
    using namespace boost::python;
    def("hello", &hello);
    def("add_cpp", &add_cpp);
    def("isPrime", &isPrime);
}

