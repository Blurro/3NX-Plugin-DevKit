#pragma once

// Standard section annotations for Nexus3DS sysplugins.
// The 4-character id token must match the plugin ID configured in makeplugin.sh.
#if defined(__GNUC__)
#define PLUGIN_MAIN(id)   __attribute__((section(".plugin_" #id "_entry"), used))
#define PLUGIN_CODE(id)   __attribute__((section(".plugin_" #id), used))
#define PLUGIN_RODATA(id) __attribute__((section(".pluginrodata_" #id), used))
#define PLUGIN_DATA(id)   __attribute__((section(".plugindata_" #id), used))
#define PLUGIN_BSS(id)    __attribute__((section(".pluginbss_" #id), used))
#else
#define PLUGIN_MAIN(id)
#define PLUGIN_CODE(id)
#define PLUGIN_RODATA(id)
#define PLUGIN_DATA(id)
#define PLUGIN_BSS(id)
#endif
