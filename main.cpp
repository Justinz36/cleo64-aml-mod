#include <android/log.h>

#define LOG_TAG "CLEO64"

#define LOGI(...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" __attribute__((visibility("default")))
void cleo64_test_log() {
    LOGI("CLEO64 ARM64 test function called");
}

__attribute__((constructor))
static void on_library_loaded() {
    LOGI("CLEO64 library loaded successfully");
}
