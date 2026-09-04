Nexus3DS Sysplugin Dev Kit
==========================

Base workflow
-------------
Apply this ZIP over a clean, unbuilt sysplugin-capable stock Nexus3DS source tree.
Then add a plugin overlay, edit only the CONFIG block at the top of makeplugin.sh,
and run ./makeplugin.sh. No preliminary top-level make is required.

On the first plugin build, makeplugin.sh generates
sysplugin/include/SysPluginLoaderEntryGenerated.h on demand using stock
sysplugin/build_pair.py. Temporary K11 build files and k11_extension.elf created
for header generation are removed afterward. The generated header remains in the
working tree for later plugin builds; it is generated state and must not be
shipped in plugin overlays.

Dev-kit ownership contract
--------------------------
Files listed in SYSPLUGIN_DEVKIT_OWNED_FILES.txt belong to the dev kit.
Future plugin overlays MUST NOT replace or modify any of those files, except
that the four configuration arrays inside the delimited CONFIG block of
makeplugin.sh may be changed:

    ROSALINA_PLUGIN_CONFIG
    LOADER_PLUGIN_CONFIG
    METADATA_CONFIG
    STACKED_PLUGIN_CONFIG

Do not ship a plugin-specific copy of a dev-kit-owned helper/header/linker file.
If generic infrastructure needs a new capability, update the dev kit itself.

Plugin overlays should contain only plugin-owned source/assets/scripts plus the
makeplugin.sh config edit. Plugin-specific pre_makeplugin.sh scripts are fine,
but they must consume dev-kit tooling rather than vendoring/replacing it.

Standard plugin section macros
------------------------------
The shared dev-kit header sysplugin/include/sysplugin_plugin.h provides:

    PLUGIN_MAIN(id)
    PLUGIN_CODE(id)
    PLUGIN_RODATA(id)
    PLUGIN_DATA(id)
    PLUGIN_BSS(id)

New plugins may include "sysplugin_plugin.h" instead of copying these standard
section definitions into every source file. Plugin-specific extra sections may
still be defined in plugin-owned source when needed.

Cross-plugin references
-----------------------
A cross-plugin symbol names its provider directly:

    PLUGIN_abcd_Function
    PLUGIN_abcd_SomeObject

where "abcd" is the exact 4-character plugin ID. Add that ID to the consumer's
allowed_refs in makeplugin.sh. The provider implementation does NOT need to be
built in the same run; create3nx records a runtime provider repair for a valid
undefined PLUGIN_abcd_* import.

The symbol kind is part of the runtime repair hash, so public provider headers
must type imported symbols correctly. This devkit supplies sysplugin_symbols.h:

    extern bool PLUGIN_abcd_Function(u32 value);
    NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_abcd_Function);

    extern u32 PLUGIN_abcd_State;
    NEXUS_PLUGIN_EXTERNAL_OBJECT(PLUGIN_abcd_State);

For relocatable cross-provider function calls, store the symbol address in the
consumer plugin's import table and call through that repaired pointer. Do not
emit a direct ARM branch to an external provider. Example:

    PLUGIN_DATA(test) void *pluginTable_test[] = {
        (void *)PLUGIN_abcd_Function,
    };
    #define TEST_ABCD_Function ((bool(*)(u32))pluginTable_test[0])

The provider's own PLUGIN_abcd_* definition is exported automatically by the
3NX generator when it belongs to that plugin's sections. allowed_refs defines
allowed cross-plugin references.

MENU symbols
------------
Rosalina builds include sysplugin_menu.h as the dev-kit-owned public MENU ABI.
Consumers only need:

    #include "sysplugin_menu.h"

plus MENU in that Rosalina plugin's allowed_refs. MENU implementation source is
not required to build the consumer. The header currently exposes public API
revision 2, including PLUGIN_MENU_OpenOnlineSource().

Loader plugins cannot reference Rosalina MENU because Loader and Rosalina are
separate sysmodules with their own plugin environments.

Transient .3on tooling
----------------------
The dev kit owns the generic transient tools:

    sysplugin/make_transient_3on.py
    sysplugin/compress_3on_lzss.py

Plugin overlays that build .3on files should call these tools; do not include
private copies. make_transient_3on.py converts an eligible single Rosalina .3nx
entry into 3NX& transient form. compress_3on_lzss.py wraps a versioned transient
using the existing 3NXO/LZ10 format.
