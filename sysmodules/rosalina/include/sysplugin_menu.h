/**
 * @file sysplugin_menu.h
 * @brief MENU sysplugin API for Rosalina plugins.
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
/// Bridge API revision.
#define SYSPLUGIN_MENU_BRIDGE_API_REVISION 1u
/// Sysplugin management API revision.
#define SYSPLUGIN_MENU_MANAGE_API_REVISION 1u
/// Maximum bridge message payload size in bytes.
#define SYSPLUGIN_MENU_BRIDGE_MAX_PAYLOAD 0xC0u

/// Menu item registered by a plugin.
typedef struct PluginMenuRegistration
{
    u32 pluginId; ///< ID of the plugin that owns the item.
    const char *title; ///< Displayed item title.
    void (*callback)(void); ///< Called when the item is selected.
    u32 color; ///< Menu item color value.
    struct PluginMenuRegistration *next; ///< Used by MENU to link registrations.
} PluginMenuRegistration;

/// Open plugin file and selected entry information.
typedef struct PluginMenuFileContext
{
    FS_Archive archive; ///< SD archive used by the open file.
    Handle file; ///< Open .3nx file handle.
    u32 entryOffset; ///< Selected .3nx entry's header offset in the file.
    u32 metadataOffset; ///< Selected entry's metadata offset in the file.
    u32 metadataSize; ///< Selected entry's metadata size in bytes.
} PluginMenuFileContext;

/// Bridge receiver callback; return true when the message was handled, or false to request a retry.
typedef bool (*PluginMenuBridgeReceiverCallback)(u32 command, const void *payload, u32 payloadSize);

/// Bridge receiver registered by a plugin.
typedef struct PluginMenuBridgeRegistration
{
    u32 pluginId; ///< ID receiving bridge messages.
    PluginMenuBridgeReceiverCallback callback; ///< Called for messages addressed to pluginId.
    struct PluginMenuBridgeRegistration *next; ///< Used by MENU to link registrations.
} PluginMenuBridgeRegistration;

/**
 * @brief Gets MENU's packed Loader and public API revisions.
 *
 * @return The API revision pair.
 */
u32 PLUGIN_MENU_GetApiVersion(void);

/**
 * @brief Adds or updates an item in the Rosalina sysplugin menu.
 *
 * @param item Registration to keep alive until it is removed.
 * @param pluginId Nonzero owner plugin ID.
 * @param title Item title, kept alive while registered.
 * @param callback Function called when the item is selected.
 * @param color Menu item color value.
 *
 * @return true if added or updated, false otherwise.
 */
bool PLUGIN_MENU_AddItem(PluginMenuRegistration *item, u32 pluginId, const char *title, void (*callback)(void), u32 color);

/**
 * @brief Removes a previously registered menu item.
 *
 * @param item Registration passed to PLUGIN_MENU_AddItem.
 *
 * @return true if removed, false if the item is not registered or cannot be removed.
 */
bool PLUGIN_MENU_RemoveItem(PluginMenuRegistration *item);

/**
 * @brief Gets the size of a plugin's saved MENU data.
 *
 * @param pluginId Nonzero plugin ID.
 * @param[out] sizeOut Receives the stored size on success.
 *
 * @return true if an entry was found, false otherwise.
 */
bool PLUGIN_MENU_GetDataSize(u32 pluginId, u32 *sizeOut);

/**
 * @brief Loads a plugin's saved MENU data.
 *
 * @param pluginId Nonzero plugin ID.
 * @param[out] data Destination buffer; may be NULL when size is zero.
 * @param size Buffer size in bytes; must match the stored entry size.
 *
 * @return true if the entry was read, false otherwise.
 */
bool PLUGIN_MENU_LoadData(u32 pluginId, void *data, u32 size);

/**
 * @brief Saves or replaces a plugin's MENU data.
 *
 * @param pluginId Nonzero plugin ID.
 * @param data Source buffer; may be NULL when size is zero.
 * @param size Number of bytes to save.
 *
 * @return true if saved, false otherwise.
 */
bool PLUGIN_MENU_SaveData(u32 pluginId, const void *data, u32 size);

/**
 * @brief Opens the selected Rosalina .3nx entry for a plugin ID.
 *
 * @note Close a successful context with PLUGIN_MENU_ClosePluginFile.
 *
 * @param pluginId Nonzero ID of the plugin entry to find.
 * @param[out] context Receives the open file and entry offsets on success.
 *
 * @return true if a selected entry was opened, false otherwise.
 */
bool PLUGIN_MENU_OpenPluginFile(u32 pluginId, PluginMenuFileContext *context);

/**
 * @brief Copies a byte range from an open plugin file to an SD file.
 *
 * @note The source range is checked against the file size. A partial destination is removed on failure.
 *
 * @param source Open plugin file context.
 * @param sourceOffset Absolute byte offset in the open file.
 * @param sourceSize Number of bytes to copy.
 * @param outputPath Destination path on the SD archive; an existing file is replaced.
 *
 * @return Zero on success, an error Result otherwise.
 */
Result PLUGIN_MENU_ExtractRawFile(const PluginMenuFileContext *source, u32 sourceOffset, u32 sourceSize, const char *outputPath);

/**
 * @brief Decompresses an LZ10 byte range from an open plugin file to an SD file.
 *
 * @param source Open plugin file context.
 * @param compressedOffset Absolute byte offset of the LZ10 stream.
 * @param compressedSize Size of the compressed stream in bytes.
 * @param outputPath Destination path on the SD archive; an existing file is replaced.
 *
 * @return Zero on success, an error Result otherwise.
 */
Result PLUGIN_MENU_UnpackLz10File(const PluginMenuFileContext *source, u32 compressedOffset, u32 compressedSize, const char *outputPath);

/**
 * @brief Closes an open plugin file and clears its context.
 *
 * @param context Context returned by PLUGIN_MENU_OpenPluginFile.
 */
void PLUGIN_MENU_ClosePluginFile(PluginMenuFileContext *context);

/**
 * @brief Adds a titled source to the Online Menu.
 *
 * @param title Entry title.
 * @param url Source URL.
 *
 * @return true if added or already present, false otherwise.
 */
bool PLUGIN_MENU_AddOnlineEntry(const char *title, const char *url);

/**
 * @brief Removes Online Menu entries with a given title.
 *
 * @param title Title of the entries to remove.
 *
 * @return true if removed or not present, false on invalid input or a file error.
 */
bool PLUGIN_MENU_RemoveOnlineEntry(const char *title);

/**
 * @brief Opens an Online Menu source URL.
 *
 * @param url Nonempty source URL.
 */
void PLUGIN_MENU_OpenOnlineSource(const char *url);

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
 * @brief Registers a receiver for a plugin's bridge messages.
 *
 * @note Keep the registration alive until PLUGIN_MENU_UnregisterBridgeReceiver. Only one receiver may use each plugin ID.
 *
 * @param registration Registration with a nonzero plugin ID and a callback.
 *
 * @return true if registered, false if invalid or the ID or registration is already present.
 */
bool PLUGIN_MENU_RegisterBridgeReceiver(PluginMenuBridgeRegistration *registration);

/**
 * @brief Removes a bridge receiver registration.
 *
 * @param registration Previously registered receiver.
 *
 * @return true if removed, false if it was not registered.
 */
bool PLUGIN_MENU_UnregisterBridgeReceiver(PluginMenuBridgeRegistration *registration);

/**
 * @brief Adds an existing .3nx file to MENU's pending sysplugin list.
 *
 * @param name Filename or path ending in .<priority>.3nx or .<priority>.3nx.d.
 *
 * @return true if the existing file was recorded, false otherwise.
 */
bool PLUGIN_MENU_AddSysplugin(const char *name);

/**
 * @brief Disables a sysplugin by renaming its .3nx file to .3nx.d.
 *
 * @param name Priority-numbered .3nx filename or path.
 *
 * @return true if renamed and the state was saved, false otherwise.
 */
bool PLUGIN_MENU_DisableSysplugin(const char *name);

/**
 * @brief Enables a sysplugin by removing the .d suffix from its filename.
 *
 * @param name Priority-numbered .3nx.d filename or path.
 *
 * @return true if renamed and the state was saved, false otherwise.
 */
bool PLUGIN_MENU_EnableSysplugin(const char *name);

/**
 * @brief Deletes a sysplugin file from the plugins directory.
 *
 * @param name Priority-numbered .3nx or .3nx.d filename or path.
 *
 * @return true if the file was deleted and the state was saved, false otherwise.
 */
bool PLUGIN_MENU_DeleteSysplugin(const char *name);

/// Marks the imported MENU symbols as ELF functions for 3NX repair.
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_GetApiVersion);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_AddItem);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RemoveItem);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_GetDataSize);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_LoadData);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_SaveData);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_OpenPluginFile);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_ExtractRawFile);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnpackLz10File);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_ClosePluginFile);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_AddOnlineEntry);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RemoveOnlineEntry);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_OpenOnlineSource);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_FindFreeRange);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_TempAlloc);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_TempFree);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_MapPage);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnmapPage);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RegisterBridgeReceiver);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_UnregisterBridgeReceiver);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_AddSysplugin);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_DisableSysplugin);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_EnableSysplugin);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_DeleteSysplugin);
