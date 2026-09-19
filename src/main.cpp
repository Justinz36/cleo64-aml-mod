#include <android/log.h>

#include <mod/amlmod.h>
#include <mod/logger.h>

#define CLEO64_TAG "CLEO64"

#define CLEO64_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, CLEO64_TAG, __VA_ARGS__)

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.2,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

// จุดนี้จะทำงานทันทีเมื่อระบบโหลด libcleo64.so
__attribute__((constructor))
static void cleo64_library_loaded()
{
    CLEO64_LOG("================================");
    CLEO64_LOG("CLEO64: libcleo64.so loaded");
    CLEO64_LOG("CLEO64: native constructor reached");
    CLEO64_LOG("CLEO64: ABI arm64-v8a build");
    CLEO64_LOG("CLEO64: waiting for AML OnModLoad");
    CLEO64_LOG("================================");
}

// จุดนี้จะทำงานเมื่อ AML โหลดและเรียก mod ของเรา
extern "C" void OnModLoad()
{
    CLEO64_LOG("CLEO64: OnModLoad entered");
    CLEO64_LOG("CLEO64: AML mod callback reached");
    CLEO64_LOG("CLEO64: package com.rockstargames.gtasa");
    CLEO64_LOG("CLEO64: mod initialization started");

    if (logger != nullptr)
    {
        logger->SetTag("CLEO64");
        logger->Info("CLEO64 AML ARM64 mod loaded successfully");
        logger->Info("CLEO64 log test passed");
    }

    CLEO64_LOG("CLEO64: mod initialization finished");
}
