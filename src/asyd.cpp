#include "config.hpp"

using namespace asyd;

int main(int argc, char** argv)
{
    Config config(ConfigType::SERVER);
    config.to_file("test.cfg");

    return 0;
}
