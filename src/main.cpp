#include "util/log.h"
#include "util/input_parser.h"
#include "util/file_util.h"
#include "lua/environment.h"

#include <string>

using namespace hop;

void show_usage()
{
    std::cout << "usage: hop -h\n"
              << "usage: hop -s script.lua\n"
              << "\n"
              << "options:\n"
              << "       -h  Print this menu\n"
              << "       -s  Run a lua script\n"
              << "       -v  Verbose\n"
              << "       -vv Very verbose\n" << std::endl;
}

int main(int argc, char* argv[])
{
    Log::set_log_level(INFO);

    try
    {
        Log("hop") << INFO << "Hop - Path tracing renderer";

        InputParser input(argc, argv);

        if (input.option_exists("-v"))
            Log::set_log_level(INFO);

        if (input.option_exists("-vv"))
            Log::set_log_level(DEBUG);

        if (input.option_exists("-h") || argc == 1)
        {
            show_usage();
        }
        else if (input.option_exists("-s"))
        {
            const std::string& file = input.get_option("-s");
            if (!has_extension(file, ".lua"))
            {
                Log("main") << WARNING << file << " is not a lua file";
            }
            else
            {
                lua::Environment env;
                env.load(file.c_str());
                env.call("init", "");
            }
        }
        else
        {
            Log("main") << WARNING << "unknown option " << input.get_options()[0];

            show_usage();
        }
    }
    catch (std::exception& e)
    {
        Log("main") << ERROR << e.what();
    }

    return 0;
}
