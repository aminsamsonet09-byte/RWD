// RWD COLOR LAB - PLUG-IN LOAD TEST (not a Resolve test).
//
// Loads the built .ofx binary the way any OFX host does (dlopen / LoadLibrary),
// resolves the two mandatory OFX exports and checks the plug-in descriptor.
// It proves the binary is a loadable OFX module with the right identity. It does
// NOT prove that DaVinci Resolve accepts or renders it.
//
// usage: plugin_load_test <path-to-RWD_COLOR_LAB.ofx>

#include <cstdio>
#include <cstring>

#include "ofxCore.h"
#include "ofxImageEffect.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static int g_failed = 0;
#define EXPECT(cond, msg)                                              \
    do {                                                               \
        if (!(cond)) { std::printf("FAIL: %s\n", msg); ++g_failed; }   \
        else { std::printf(" ok : %s\n", msg); }                       \
    } while (0)

typedef int (*GetNumFn)(void);
typedef OfxPlugin* (*GetPluginFn)(int);

int main(int argc, char** argv) {
    if (argc < 2) { std::printf("usage: %s <plugin.ofx>\n", argv[0]); return 2; }

#ifdef _WIN32
    HMODULE lib = LoadLibraryA(argv[1]);
    auto sym = [&](const char* n) { return reinterpret_cast<void*>(GetProcAddress(lib, n)); };
#else
    void* lib = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    auto sym = [&](const char* n) { return dlsym(lib, n); };
#endif
    EXPECT(lib != nullptr, "binary loads as a shared library");
    if (!lib) return 1;

    GetNumFn getNum = reinterpret_cast<GetNumFn>(sym("OfxGetNumberOfPlugins"));
    GetPluginFn getPlugin = reinterpret_cast<GetPluginFn>(sym("OfxGetPlugin"));
    EXPECT(getNum != nullptr, "exports OfxGetNumberOfPlugins");
    EXPECT(getPlugin != nullptr, "exports OfxGetPlugin");
    if (!getNum || !getPlugin) return 1;

    EXPECT(getNum() == 1, "reports exactly 1 plug-in");
    OfxPlugin* p = getPlugin(0);
    EXPECT(p != nullptr, "OfxGetPlugin(0) returns a descriptor");
    // Note: OfxGetPlugin(n >= count) is undefined in the official Support library; hosts must not call it.
    if (!p) return 1;

    EXPECT(p->pluginApi && std::strcmp(p->pluginApi, kOfxImageEffectPluginApi) == 0, "plug-in API is OfxImageEffectPluginAPI");
    EXPECT(p->apiVersion == 1, "plug-in API version is 1");
    EXPECT(p->pluginIdentifier && std::strcmp(p->pluginIdentifier, "id.ridwanard.RWDColorLab") == 0, "identifier is id.ridwanard.RWDColorLab");
    std::printf("      version %u.%u\n", p->pluginVersionMajor, p->pluginVersionMinor);
    EXPECT(p->pluginVersionMajor == 0 && p->pluginVersionMinor == 3, "version is 0.3");
    EXPECT(p->setHost != nullptr, "has setHost");
    EXPECT(p->mainEntry != nullptr, "has mainEntry");

#ifdef _WIN32
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
    std::printf("\n%s (%d failed)\n", g_failed ? "FAILED" : "PASSED", g_failed);
    return g_failed ? 1 : 0;
}
