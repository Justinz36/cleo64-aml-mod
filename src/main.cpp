#include <android/log.h>

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <strings.h>

#include <string>
#include <vector>

#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/interface.h>

#include <imgui/iimgui.h>

#define CLEO64_TAG "CLEO64"

#define CLEO64_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, CLEO64_TAG, __VA_ARGS__)

MYMOD(
    net.justinz36.cleo64,
    CLEO64,
    0.9,
    JustinZ36
)

NEEDGAME(com.rockstargames.gtasa)

BEGIN_DEPLIST()
    ADD_DEPENDENCY(net.rusjj.imgui)
END_DEPLIST()

struct ScriptEntry
{
    std::string name;
    bool autoStart;
};

static std::vector<ScriptEntry> g_scripts;
static IImGui* g_imgui = nullptr;
static bool g_menuOpen = true;
static int g_selectedScript = -1;
static bool g_listenerRegistered = false;

static const char* get_game_directory()
{
    return
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa";
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

    const char* directoryPath = get_game_directory();
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

        if (entry->d_type == DT_DIR)
            continue;

        const bool isCsi = has_extension(filename, ".csi");
        const bool isCsa = has_extension(filename, ".csa");

        if (!isCsi && !isCsa)
            continue;

        ScriptEntry script;
        script.name = filename;
        script.autoStart = isCsa;

        g_scripts.push_back(script);
    }

    closedir(directory);

    CLEO64_LOG(
        "CLEO64 found %zu script(s)",
        g_scripts.size()
    );
}

static void write_selection_marker()
{
    const char* markerPath =
        "/storage/emulated/0/Android_unprotected/data/"
        "com.rockstargames.gtasa/cleo64_imgui_selection.txt";

    FILE* file = std::fopen(markerPath, "w");

    if (file == nullptr)
    {
        CLEO64_LOG("Could not write selection marker");
        return;
    }

    std::fprintf(file, "CLEO64 ImGui selection\n");

    if (g_selectedScript >= 0 &&
        g_selectedScript <
            static_cast<int>(g_scripts.size()))
    {
        std::fprintf(
            file,
            "Selected: %s\n",
            g_scripts[g_selectedScript].name.c_str()
        );
    }
    else
    {
        std::fprintf(file, "Selected: none\n");
    }

    std::fclose(file);
}

static void render_cleo64_menu()
{
    if (g_imgui == nullptr || !g_menuOpen)
        return;

    const int screenWidth = g_imgui->GetScreenSizeX();
    const int screenHeight = g_imgui->GetScreenSizeY();

    g_imgui->SetNextWindowPos(
        ImVec2(
            screenWidth * 0.08f,
            screenHeight * 0.08f
        ),
        ImGuiCond_Always
    );

    g_imgui->SetNextWindowSize(
        ImVec2(
            screenWidth * 0.84f,
            screenHeight * 0.78f
        ),
        ImGuiCond_Always
    );

    if (!g_imgui->Begin(
        "CLEO64",
        &g_menuOpen,
        ImGuiWindowFlags_NoCollapse
    ))
    {
        g_imgui->End();
        return;
    }

    g_imgui->Text("CLEO64 ARM64 - Script Menu");

    g_imgui->Text(
        "Scripts found: %d",
        static_cast<int>(g_scripts.size())
    );

    g_imgui->Separator();

    if (g_imgui->Button("Refresh"))
    {
        scan_cleo_scripts();
    }

    g_imgui->SameLine();

    if (g_imgui->Button("Close"))
    {
        g_menuOpen = false;
    }

    g_imgui->Separator();

    for (size_t i = 0; i < g_scripts.size(); ++i)
    {
        const bool selected =
            g_selectedScript == static_cast<int>(i);

        std::string label = g_scripts[i].name;

        if (g_scripts[i].autoStart)
            label += " [AUTO]";

        if (g_imgui->Selectable(
            label.c_str(),
            selected
        ))
        {
            g_selectedScript =
                static_cast<int>(i);

            write_selection_marker();

            CLEO64_LOG(
                "Selected script: %s",
                g_scripts[i].name.c_str()
            );
        }
    }

    if (g_selectedScript >= 0 &&
        g_selectedScript <
            static_cast<int>(g_scripts.size()))
    {
        g_imgui->Separator();

        g_imgui->Text(
            "Selected: %s",
            g_scripts[g_selectedScript].name.c_str()
        );

        if (g_imgui->Button("Test selected"))
        {
            CLEO64_LOG(
                "Test selected: %s",
                g_scripts[g_selectedScript].name.c_str()
            );
        }
    }

    g_imgui->End();
}

static void attach_imgui()
{
    if (g_listenerRegistered)
        return;

    g_imgui = static_cast<IImGui*>(
        GetInterface("ImGui")
    );

    if (g_imgui == nullptr)
    {
        CLEO64_LOG(
            "AML_ImGui interface not found"
        );
        return;
    }

    g_imgui->AddRenderListener(
        reinterpret_cast<void*>(
            &render_cleo64_menu
        )
    );

    g_listenerRegistered = true;

    CLEO64_LOG(
        "CLEO64 attached to AML_ImGui"
    );
}

ON_MOD_LOAD()
{
    CLEO64_LOG(
        "CLEO64 OnModLoad started"
    );

    if (logger != nullptr)
    {
        logger->SetTag("CLEO64");
        logger->Info(
            "CLEO64 ImGui consumer loading"
        );
    }

    scan_cleo_scripts();
    attach_imgui();

    if (logger != nullptr)
    {
        if (g_imgui != nullptr)
        {
            logger->Info(
                "AML_ImGui interface found"
            );
        }
        else
        {
            logger->Error(
                "AML_ImGui interface not found"
            );
        }
    }
}
