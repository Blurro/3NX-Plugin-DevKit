#pragma once

/*
 * Cross-plugin imports are resolved by 3NX provider ID from symbols named:
 *     PLUGIN_<4-char provider ID>_*
 *
 * The runtime repair hash includes the ELF symbol kind, so an external symbol
 * must be explicitly typed in the consumer ELF. Put these declarations in the
 * provider's public header alongside the normal C declaration.
 */
#if defined(__GNUC__)
#define NEXUS_PLUGIN_EXTERNAL_FUNC(symbol)   __asm__(".type " #symbol ", %function")
#define NEXUS_PLUGIN_EXTERNAL_OBJECT(symbol) __asm__(".type " #symbol ", %object")
#else
#define NEXUS_PLUGIN_EXTERNAL_FUNC(symbol)
#define NEXUS_PLUGIN_EXTERNAL_OBJECT(symbol)
#endif
