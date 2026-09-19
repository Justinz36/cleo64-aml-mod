#include <android/log.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <string>
#include <vector>

#include <mod/amlmod.h>
#include <mod/logger.h>

#define CLEO64_TAG "CLEO64"

#define CLEO64_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, CLEO64_TAG, __VA_ARGS__)

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.6,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

struct ScriptEntry
{
    std::string name;
    std::string path;
    bool autoStart;
};

static std::vector<ScriptEntry> g_scripts;

static const char* get_script_directory()
{
    return "/storage/emulated/0/Android_unprotected/data/"
           "com.rockstargames.gtasa/files/mods/cleo";
}

static bool has_extension(const char* name, const char* extension)
{
    if (name == nullptr || extension == nullptr)
        return false;

    const char* dot = std::strrchr(name, '.');

    if (dot == nullptr)
        return false;

    return strcasecmp(dot, extension) == 0;
}

static void scan_cleo_scripts()
{
    g_scripts.clear();

    const char* directory_path = get_script_directory();
    DIR* directory = opendir(directory_path);

    if (directory == nullptr)
    {
        CLEO64_LOG("Script directory not found: %s", directory_path);
        return;
    }

    dirent* entry = nullptr;

    while ((entry = readdir(directory)) != nullptr)
    {
        if (entry->d_type == DT_DIR)
            continue;

        const char* filename = entry->d_name;

        if (has_extension(filename, ".csi"))
        {
            g_scripts.push_back({
                filename,
                std::string(directory_path) + "/" + filename,
                false
            });
        }
        else if (has_extension(filename, ".csa"))
        {
            g_scripts.push_back({
                filename,
                std::string(directory_path) + "/" + filename,
                true
            });
        }
    }

    closedir(directory);

    CLEO64_LOG("Script scan complete: %zu script(s)", g_scripts.size());

    for (size_t i = 0; i < g_scripts.size(); ++i)
    {
        CLEO64_LOG(
            "Menu item %zu: %s%s",
            i + 1,
            g_scripts[i].name.c_str(),
            g_scripts[i].autoStart ? " [AUTO]" : ""
        );
    }
}

static void write_menu_marker()
{
    const char* marker_path =
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa/files/cleo64_menu.txt";

    FILE* file = std::fopen(marker_path, "w");

    if (file == nullptr)
    {
        CLEO64_LOG("Could not write menu marker");
        return;
    }

    std::fprintf(file, "CLEO64 menu backend ready\n");
    std::fprintf(file, "ABI: arm64-v8a\n");
    std::fprintf(file, "Package: com.rockstargames.gtasa\n");
    std::fprintf(file, "Script directory: %s\n", get_script_directory());
    std::fprintf(file, "Script count: %zu\n\n", g_scripts.size());

    for (size_t i = 0; i < g_scripts.size(); ++i)
    {
        std::fprintf(
            file,
            "%zu. %s%s\n",
            i + 1,
            g_scripts[i].name.c_str(),
            g_scripts[i].autoStart ? " [AUTO]" : ""
        );
    }

    std::fclose(file);

    CLEO64_LOG("Menu marker written: %s", marker_path);
}

ON_MOD_LOAD()
{
    CLEO64_LOG("CLEO64 menu backend starting");

    if (logger != nullptr)
    {
        logger->SetTag("CLEO64");
        logger->Info("CLEO64 menu backend loaded");
    }

    scan_cleo_scripts();
    write_menu_marker();

    if (logger != nullptr)
    {
        logger->Info(
            "CLEO64 found %zu script(s)",
            g_scripts.size()
        );
    }
}
