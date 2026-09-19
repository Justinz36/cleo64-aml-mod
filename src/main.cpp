#include <android/log.h>
#include <cstdio>

#include <mod/amlmod.h>
#include <mod/logger.h>

#define CLEO64_TAG "CLEO64"

#define CLEO64_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, CLEO64_TAG, __VA_ARGS__)

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.4,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

static void write_loaded_marker(const char* stage)
{
    const char* paths[] = {
        "/storage/emulated/0/Android_unprotected/data/com.rockstargames.gtasa/files/cleo64_loaded.txt",
        "/storage/emulated/0/Android/data/com.rockstargames.gtasa/files/cleo64_loaded.txt"
    };

    for (const char* path : paths)
    {
        FILE* file = std::fopen(path, "w");

        if (file != nullptr)
        {
            std::fprintf(
                file,
                "CLEO64 loaded successfully\n"
                "Stage: %s\n"
                "ABI: arm64-v8a\n"
                "Package: com.rockstargames.gtasa\n",
                stage
            );

            std::fclose(file);

            CLEO64_LOG("Marker updated: %s", path);
            CLEO64_LOG("Stage: %s", stage);
            return;
        }
    }

    CLEO64_LOG("Could not write cleo64_loaded.txt");
}

__attribute__((constructor))
static void cleo64_library_loaded()
{
    CLEO64_LOG("libcleo64.so loaded");
    write_loaded_marker("constructor");
}

extern "C" void OnModLoad()
{
    CLEO64_LOG("OnModLoad entered");

    // เขียนทับ marker จาก constructor ด้วย Stage: OnModLoad
    write_loaded_marker("OnModLoad");

    if (logger != nullptr)
    {
        logger->SetTag("CLEO64");
        logger->Info("CLEO64 AML ARM64 mod loaded successfully");
        logger->Info("CLEO64 OnModLoad marker written");
    }

    CLEO64_LOG("CLEO64 initialization finished");
}
