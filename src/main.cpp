#include <mod/amlmod.h>
#include <mod/logger.h>

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.1,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

ON_MOD_LOAD()
{
    logger->SetTag("CLEO64");
    logger->Info("CLEO64 AML mod loaded");
}
