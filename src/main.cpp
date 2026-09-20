#include <android/log.h>

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <strings.h>

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
    0.8,
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

static bool g_menuOpen = false;
static int g_selectedScript = -1;

static const char* get_game_directory()
{
    return
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa";
}

static const char* get_script_directory()
{
    return get_game_directory();
}

static const char* get_menu_marker_path()
{
    return
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa/cleo64_menu.txt";
}

static const char* get_menu_state_path()
{
    return
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa/cleo64_menu_state.txt";
}

static bool has_extension(
    const char* filename,
    const char* extension
)
{
    if (filename == nullptr || extension == nullptr)
        return false;

    const char* dot = std::strrchr(filename, '.');

    if (dot == nullptr)
        return false;

    return strcasecmp(dot, extension) == 0;
}

static void scan_cleo_scripts()
{
    g_scripts.clear();

    const char* directoryPath = get_script_directory();
    DIR* directory = opendir(directoryPath);

    if (directory == nullptr)
    {
        CLEO64_LOG(
            "Script directory not found: %s",
            directoryPath
        );
        return;
    }

    dirent* entry = nullptr;

    while ((entry = readdir(directory)) != nullptr)
    {
        const char* filename = entry->d_name;

        if (filename == nullptr)
            continue;

        if (std::strcmp(filename, ".") == 0 ||
            std::strcmp(filename, "..") == 0)
        {
            continue;
        }

        // ค้นหาเฉพาะไฟล์ที่อยู่ตรงในโฟลเดอร์เกมหลัก
        // ไม่ค้นหาใน configs, files หรือ mods
        if (entry->d_type == DT_DIR)
            continue;

        const bool isCsi = has_extension(filename, ".csi");
        const bool isCsa = has_extension(filename, ".csa");

        if (!isCsi && !isCsa)
            continue;

        ScriptEntry script;
        script.name = filename;
        script.path =
            std::string(directoryPath) + "/" + filename;
        script.autoStart = isCsa;

        g_scripts.push_back(script);
    }

    closedir(directory);

    CLEO64_LOG(
        "Script scan complete: %zu script(s)",
        g_scripts.size()
    );

    for (size_t i = 0; i < g_scripts.size(); ++i)
    {
        CLEO64_LOG(
            "Script %zu: %s%s",
            i + 1,
            g_scripts[i].name.c_str(),
            g_scripts[i].autoStart ? " [AUTO]" : ""
        );
    }
}

static void write_menu_marker()
{
    const char* markerPath = get_menu_marker_path();

    FILE* file = std::fopen(markerPath, "w");

    if (file == nullptr)
    {
        CLEO64_LOG(
            "Could not write menu marker: %s",
            markerPath
        );
        return;
    }

    std::fprintf(
        file,
        "CLEO64 menu backend ready\n"
        "ABI: arm64-v8a\n"
        "Package: com.rockstargames.gtasa\n"
        "Script directory: %s\n"
        "Script count: %zu\n\n",
        get_script_directory(),
        g_scripts.size()
    );

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

    CLEO64_LOG(
        "Menu marker written: %s",
        markerPath
    );
}

static void write_menu_state()
{
    const char* statePath = get_menu_state_path();

    FILE* file = std::fopen(statePath, "w");

    if (file == nullptr)
    {
        CLEO64_LOG(
            "Could not write menu state: %s",
            statePath
        );
        return;
    }

    std::fprintf(file, "CLEO64 menu test\n");
    std::fprintf(
        file,
        "Menu: %s\n",
        g_menuOpen ? "OPEN" : "CLOSED"
    );
    std::fprintf(
        file,
        "Selected: %d\n",
        g_selectedScript
    );

    if (g_selectedScript >= 0 &&
        g_selectedScript <
            static_cast<int>(g_scripts.size()))
    {
        std::fprintf(
            file,
            "Selected script: %s\n",
            g_scripts[g_selectedScript].name.c_str()
        );
    }

    std::fprintf(file, "\nScripts:\n");

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

    CLEO64_LOG(
        "Menu state written: %s",
        g_menuOpen ? "OPEN" : "CLOSED"
    );
}

static void open_menu_test()
{
    g_menuOpen = true;
    g_selectedScript = -1;

    CLEO64_LOG("Test menu opened");
    write_menu_state();
}

static void close_menu_test()
{
    g_menuOpen = false;

    CLEO64_LOG("Test menu closed");
    write_menu_state();
}

static void select_script_test(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(g_scripts.size()))
    {
        CLEO64_LOG(
            "Invalid script index: %d",
            index
        );
        return;
    }

    g_selectedScript = index;

    CLEO64_LOG(
        "Selected script: %s",
        g_scripts[index].name.c_str()
    );

    write_menu_state();
}

__attribute__((constructor))
static void cleo64_library_loaded()
{
    CLEO64_LOG("==============================");
    CLEO64_LOG("CLEO64 library loaded");
    CLEO64_LOG("ABI: arm64-v8a");
    CLEO64_LOG("==============================");
}

ON_MOD_LOAD()
{
    CLEO64_LOG("CLEO64 OnModLoad entered");

    if (logger != nullptr)
    {
        logger->SetTag("CLEO64");
        logger->Info("CLEO64 menu backend starting");
    }

    scan_cleo_scripts();
    write_menu_marker();

    // เปิดสถานะเมนูทดสอบ
    // ตอนนี้ยังเป็น backend marker ยังไม่ใช่ UI บนหน้าจอ
    open_menu_test();

    if (logger != nullptr)
    {
        logger->Info(
            "CLEO64 found %zu script(s)",
            g_scripts.size()
        );

        logger->Info(
            "CLEO64 menu backend ready"
        );
    }

    CLEO64_LOG(
        "CLEO64 initialization finished"
    );
}
