/**
 * @file sysplugin_menu.h
 * @brief MENU sysplugin API for Loader plugins.
 *
 * @note Add MENU to allowed_refs in makeplugin.sh before importing these functions.
 */

#pragma once

#include <3ds.h>
#include "sysplugin_symbols.h"

/// The four-character MENU provider ID, encoded as a 32-bit integer.
#define SYSPLUGIN_MENU_PROVIDER_ID 0x554E454Du
/// Loader API revision.
#define SYSPLUGIN_MENU_LOADER_API_REVISION 2u
/// Public API revision.
#define SYSPLUGIN_MENU_PUBLIC_API_REVISION 3u
/// Public revision in the upper 16 bits and Loader revision in the lower 16 bits.
#define SYSPLUGIN_MENU_API_REVISION_PAIR \
    (SYSPLUGIN_MENU_LOADER_API_REVISION | (SYSPLUGIN_MENU_PUBLIC_API_REVISION << 16))
/// Extracts the Loader revision from an API version pair.
#define SYSPLUGIN_MENU_API_LOADER_REVISION(pair) ((u16)((pair) & 0xFFFFu))
/// Extracts the public revision from an API version pair.
#define SYSPLUGIN_MENU_API_PUBLIC_REVISION(pair) ((u16)((pair) >> 16))
/// Maximum bridge message payload size in bytes.
#define SYSPLUGIN_MENU_BRIDGE_MAX_PAYLOAD 0xC0u

/// Context supplied to Loader patch callbacks.
typedef struct PluginMenuLoaderContext
{
    u64 titleId; ///< Title ID being loaded.
    u16 titleVersion; ///< Version of the title being loaded.
    u16 reserved; ///< Reserved; set to zero by MENU.
    u8 *code; ///< Pointer to the title code buffer.
    u32 codeSize; ///< Total code buffer size in bytes.
    u32 textSize; ///< Text segment size in bytes.
    u32 roSize; ///< Read-only segment size in bytes.
    u32 dataSize; ///< Data segment size in bytes.
    u32 roAddress; ///< Read-only segment address.
    u32 dataAddress; ///< Data segment address.
    CodeSetHeader *codeSet; ///< Code set header during the prepare stage; NULL at other stages.
    Handle process; ///< Created process handle when available; zero before process creation.
} PluginMenuLoaderContext;

/// Callback run before code set creation; return false to skip later title patch stages.
typedef bool (*PluginMenuLoaderPrepareCallback)(PluginMenuLoaderContext *context);
/// Callback run at a later Loader stage for an active title patch.
typedef void (*PluginMenuLoaderStageCallback)(PluginMenuLoaderContext *context);

/// Callbacks for one or more title IDs during process loading.
typedef struct PluginMenuLoaderTitlePatch
{
    const u64 *titleIds; ///< Array of title IDs matched by this registration.
    u32 titleIdCount; ///< Number of title IDs in titleIds.
    PluginMenuLoaderPrepareCallback prepare; ///< Runs before code set creation; false skips later stage callbacks for this launch.
    PluginMenuLoaderStageCallback processCreated; ///< Runs after successful process creation when this patch is active.
    PluginMenuLoaderStageCallback loaderFinished; ///< Runs when the plugin loader closes its handle for an active launch.
    struct PluginMenuLoaderTitlePatch *next; ///< Used by MENU to link registrations.
    u32 menuEpoch; ///< MENU-owned launch tracking value; do not modify while registered.
} PluginMenuLoaderTitlePatch;

/// Prepare callback invoked for HOME Menu patching.
typedef struct PluginMenuLoaderHomePatch
{
    PluginMenuLoaderPrepareCallback prepare; ///< Runs for HOME Menu; its return value is currently ignored.
    struct PluginMenuLoaderHomePatch *next; ///< Used by MENU to link registrations.
} PluginMenuLoaderHomePatch;

/**
 * @brief Gets MENU's packed Loader and public API revisions.
 *
 * @return The API revision pair.
 */
u32 PLUGIN_MENU_GetApiVersion(void);

/**
 * @brief Finds a free address range. This does not reserve it.
 *
 * @param size Required range size in bytes.
 * @param[out] outBase Receives the start address on success.
 *
 * @return true if a range was found, false otherwise.
 */
bool PLUGIN_MENU_FindFreeRange(u32 size, u32 *outBase);

/**
 * @brief Allocates temporary read/write memory in MENU's scratch address range.
 *
 * @param size Allocation size in bytes; must be nonzero and page-aligned.
 * @param[out] outBase Receives the allocated address on success.
 *
 * @return true on success, false otherwise.
 */
bool PLUGIN_MENU_TempAlloc(u32 size, u32 *outBase);

/**
 * @brief Frees a temporary allocation.
 *
 * @param base Address returned by PLUGIN_MENU_TempAlloc.
 * @param size Size passed to PLUGIN_MENU_TempAlloc.
 */
void PLUGIN_MENU_TempFree(u32 base, u32 size);

/**
 * @brief Maps one page from a source process into the current process.
 *
 * Use this instead of calling svcMapProcessMemoryEx directly when mapping a
 * single address. MENU picks a free destination and puts an allocated guard
 * page on each side of the alias. Two aliases placed directly next to each
 * other can hit a freeing bug; the guard pages keep them apart. mappedAddress
 * keeps the source address's offset within the mapped page.
 *
 * @note Only one 0x1000-byte page is mapped. Unmap it using mappedBase, not mappedAddress.
 *
 * @param sourceProcess Handle of the process to map from.
 * @param sourceAddress Address in the source process.
 * @param[out] mappedBase Receives the mapped page's base address for PLUGIN_MENU_UnmapPage.
 * @param[out] mappedAddress Receives the mapped address with the original page offset applied.
 *
 * @return true on success, false otherwise.
 */
bool PLUGIN_MENU_MapPage(Handle sourceProcess, u32 sourceAddress, u32 *mappedBase, u32 *mappedAddress);

/**
 * @brief Unmaps a page from PLUGIN_MENU_MapPage.
 *
 * The two guard pages are freed if the unmap succeeds.
 *
 * @param mappedBase The mapped page base returned by PLUGIN_MENU_MapPage.
 */
void PLUGIN_MENU_UnmapPage(u32 mappedBase);

/**
 * @brief Registers callbacks for one or more title IDs.
 *
 * @note Keep the registration and title ID array alive until PLUGIN_MENU_UnregisterTitlePatch.
 *
 * @param registration Registration containing title IDs and at least one callback.
 *
 * @return true if registered, false if hooks are not ready, fields are missing, or it is already registered.
 */
bool PLUGIN_MENU_RegisterTitlePatch(PluginMenuLoaderTitlePatch *registration);

/**
 * @brief Removes a title patch registration.
 *
 * @param registration Previously registered title patch.
 *
 * @return true if removed, false if it was not registered.
 */
bool PLUGIN_MENU_UnregisterTitlePatch(PluginMenuLoaderTitlePatch *registration);

/**
 * @brief Registers a HOME Menu prepare callback.
 *
 * @note Keep the registration alive until PLUGIN_MENU_UnregisterHomePatch.
 *
 * @param registration Registration with a non-NULL prepare callback.
 *
 * @return true if registered, false if hooks are not ready, fields are missing, or it is already registered.
 */
bool PLUGIN_MENU_RegisterHomePatch(PluginMenuLoaderHomePatch *registration);

/**
 * @brief Removes a HOME Menu patch registration.
 *
 * @param registration Previously registered HOME patch.
 *
 * @return true if removed, false if it was not registered.
 */
bool PLUGIN_MENU_UnregisterHomePatch(PluginMenuLoaderHomePatch *registration);

/**
 * @brief Queues a message for a Rosalina bridge receiver.
 *
 * @note A successful return does not mean that the receiver has handled the message.
 *
 * @param targetPluginId Nonzero ID of the receiving plugin.
 * @param command Command supplied to the receiver.
 * @param payload Payload bytes; may be NULL when payloadSize is zero.
 * @param payloadSize Number of payload bytes, at most SYSPLUGIN_MENU_BRIDGE_MAX_PAYLOAD.
 *
 * @return true if queued, false if the bridge is not ready, the queue is full, or an argument is invalid.
 */
bool PLUGIN_MENU_BridgeSend(u32 targetPluginId, u32 command, const void *payload, u32 payloadSize);

/// Marks the imported MENU symbols as ELF functions for 3NX repair.
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_GetApiVersion);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_FindFreeRange);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_TempAlloc);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_TempFree);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_MapPage);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnmapPage);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RegisterTitlePatch);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnregisterTitlePatch);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RegisterHomePatch);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnregisterHomePatch);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_BridgeSend);
