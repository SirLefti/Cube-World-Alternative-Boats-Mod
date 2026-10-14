// Cube World Alpha - Alternative Boats Mod
//
// The game ships three boat models and only ever loads one of them. This mod
// puts one of the other two in its place, so the boat you summon is drawn with
// a model that is otherwise unreachable.
//
// Builds one DLL per model from one source - see target_boat1.h /
// target_boat3.h. Install exactly one of them.

// ---- build requirements -----------------------------------------------------
#include <stdint.h>   // UINTPTR_MAX - pointer width, checked portably

#if !defined(_WIN32)
#  error "Target must be Windows: this is a DLL injected into Cube.exe. Cross-compile, e.g. i686-w64-mingw32-g++."
#endif
#if UINTPTR_MAX != 0xFFFFFFFFu
#  error "Target must be 32-bit: Cube.exe is PE32."
#endif

#include <windows.h>

#if defined(TARGET_BOAT1) && defined(TARGET_BOAT3)
#  error "Pick one model: define TARGET_BOAT1 or TARGET_BOAT3, not both."
#elif defined(TARGET_BOAT1)
#  include "target_boat1.h"
#elif defined(TARGET_BOAT3)
#  include "target_boat3.h"
#else
#  error "No model selected: build with -DTARGET_BOAT1 or -DTARGET_BOAT3."
#endif

// address where the boat model name is located
static const uint32_t RVA_MODEL_NAME = 0x003114B8;
static const uint32_t NAME_LEN       = 12; // "boat2.cub" + 3 NUL bytes

// exact bytes to replace, to avoid writing at a location we do not expect
// this also guards if both mod dlls are installed at the same time
// this array is automatically padded with NUL bytes
static const BYTE EXPECTED_BYTES[NAME_LEN] = "boat2.cub";

// sizeof counts the NUL, which has to land inside the hole as well.
static_assert(sizeof(MODEL_NAME) <= NAME_LEN, "model name does not fit the string slot");

// ---- patching ---------------------------------------------------------------
static bool WriteName(BYTE* location)
{
    DWORD oldProtection;
    if (!VirtualProtect(location, NAME_LEN, PAGE_READWRITE, &oldProtection)) {
        return false;
    }
    const uint32_t length = sizeof(MODEL_NAME) - 1; // characters, without the NUL

    for (uint32_t i = 0; i < NAME_LEN; i++) {
        // write the new name and fill the rest with NUL
        location[i] = (i < length) ? (BYTE)MODEL_NAME[i] : 0;
    }
    VirtualProtect(location, NAME_LEN, oldProtection, &oldProtection);
    return true;
}

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    uint32_t base = (uint32_t)GetModuleHandle(NULL);
    BYTE* site = (BYTE*)(base + RVA_MODEL_NAME);

    // Refuse to patch anything that is not the expected Cube.exe build.
    if (memcmp(site, EXPECTED_BYTES, NAME_LEN) != 0) {
        return TRUE;
    }

    WriteName(site);
    return TRUE;
}
