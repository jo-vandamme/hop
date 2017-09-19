#include "util/log.h"
#include "lua/environment.h"

#include <string>

bool has_extension(const std::string& file, const std::string& ext)
{
    if (file.substr(file.length() - ext.length(), ext.length()) == ext)
        return true;
    return false;
}

int main(int argc, char* argv[])
{
    hop::Log::set_log_level(hop::DEBUG);

    for (int i = 1; i < argc; ++i)
    {
        if (has_extension(argv[i], ".lua"))
        {
            hop::lua::Environment env;
            env.load(argv[i]);
            env.execute("init");
        }
    }
    return 0;
}
