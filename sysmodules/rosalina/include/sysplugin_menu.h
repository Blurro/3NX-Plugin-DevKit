#pragma once

#include <3ds.h>
#include "sysplugin_symbols.h"

#define SYSPLUGIN_MENU_PROVIDER_ID 0x554E454Du
#define SYSPLUGIN_MENU_PUBLIC_API_REVISION 2u
#define SYSPLUGIN_MENU_BRIDGE_API_REVISION 1u
#define SYSPLUGIN_MENU_MANAGE_API_REVISION 1u
#define SYSPLUGIN_MENU_BRIDGE_MAX_PAYLOAD 0xC0u

typedef struct PluginMenuRegistration
{
    u32 pluginId;
    const char *title;
    void (*callback)(void);
    u32 color;
    struct PluginMenuRegistration *next;
} PluginMenuRegistration;

typedef struct PluginMenuFileContext
{
    FS_Archive archive;
    Handle file;
    u32 entryOffset;
    u32 metadataOffset;
    u32 metadataSize;
} PluginMenuFileContext;

typedef bool (*PluginMenuBridgeReceiverCallback)(u32 command, const void *payload, u32 payloadSize);

typedef struct PluginMenuBridgeRegistration
{
    u32 pluginId;
    PluginMenuBridgeReceiverCallback callback;
    struct PluginMenuBridgeRegistration *next;
} PluginMenuBridgeRegistration;

// Add MENU to allowed_refs in makeplugin.sh before importing these symbols.
bool PLUGIN_MENU_AddItem(PluginMenuRegistration *item, u32 pluginId, const char *title, void (*callback)(void), u32 color);
bool PLUGIN_MENU_RemoveItem(PluginMenuRegistration *item);

bool PLUGIN_MENU_GetDataSize(u32 pluginId, u32 *sizeOut);
bool PLUGIN_MENU_LoadData(u32 pluginId, void *data, u32 size);
bool PLUGIN_MENU_SaveData(u32 pluginId, const void *data, u32 size);

bool PLUGIN_MENU_OpenPluginFile(u32 pluginId, PluginMenuFileContext *context);
Result PLUGIN_MENU_UnpackLz10File(const PluginMenuFileContext *source, u32 compressedOffset, u32 compressedSize, const char *outputPath);
void PLUGIN_MENU_ClosePluginFile(PluginMenuFileContext *context);

bool PLUGIN_MENU_AddOnlineEntry(const char *title, const char *url);
bool PLUGIN_MENU_RemoveOnlineEntry(const char *title);
void PLUGIN_MENU_OpenOnlineSource(const char *url);

bool PLUGIN_MENU_FindFreeRange(u32 size, u32 *outBase);
bool PLUGIN_MENU_TempAlloc(u32 size, u32 *outBase);
void PLUGIN_MENU_TempFree(u32 base, u32 size);
bool PLUGIN_MENU_MapPage(Handle sourceProcess, u32 sourceAddress, u32 *mappedBase, u32 *mappedAddress);
void PLUGIN_MENU_UnmapPage(u32 mappedBase);

bool PLUGIN_MENU_RegisterBridgeReceiver(PluginMenuBridgeRegistration *registration);
bool PLUGIN_MENU_UnregisterBridgeReceiver(PluginMenuBridgeRegistration *registration);

bool PLUGIN_MENU_AddSysplugin(const char *name);
bool PLUGIN_MENU_DisableSysplugin(const char *name);
bool PLUGIN_MENU_EnableSysplugin(const char *name);
bool PLUGIN_MENU_DeleteSysplugin(const char *name);

NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_AddItem);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_RemoveItem);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_GetDataSize);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_LoadData);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_SaveData);
NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_MENU_OpenPluginFile);
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
