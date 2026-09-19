#include <mod/amlmod.h>
#include <mod/logger.h>

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.1,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

extern "C" void OnModLoad()
{
    logger->SetTag("CLEO64");
    logger->Info("CLEO64 AML ARM64 mod loaded successfully");
}
