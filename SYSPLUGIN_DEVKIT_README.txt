Nexus3DS Sysplugin Dev Kit
==========================

Base workflow
-------------
Apply this ZIP over a clean, unbuilt sysplugin-capable Nexus3DS source tree,
then apply your plugin overlay.

Edit only the CONFIG block at the top of makeplugin.sh, then run:

    ./makeplugin.sh


Plugin overlays
---------------
Files listed in SYSPLUGIN_DEVKIT_OWNED_FILES.txt belong to the dev kit and
should not be replaced by plugin overlays.

The only exception is the CONFIG block in makeplugin.sh:

    ROSALINA_PLUGIN_CONFIG
    LOADER_PLUGIN_CONFIG
    METADATA_CONFIG
    STACKED_PLUGIN_CONFIG

Outside of those configs, makeplugin.sh should stay identical to the dev kit
version.

A plugin overlay should only contain its own source, assets, preparation
scripts, and makeplugin.sh config.

If it has a pre_makeplugin.sh, run it separately before makeplugin.sh. It
should only prepare files needed for the real build, not build the final
plugin itself.


Plugin sections
---------------
Include:

    #include "sysplugin_plugin.h"

for the standard section macros:

    PLUGIN_MAIN(id)
    PLUGIN_CODE(id)
    PLUGIN_RODATA(id)
    PLUGIN_DATA(id)
    PLUGIN_BSS(id)


Cross-plugin references
-----------------------
Cross-plugin symbols use the provider's exact 4-character ID:

    PLUGIN_abcd_Function
    PLUGIN_abcd_SomeObject

Add abcd to the consumer's allowed_refs.

The provider does not need to be built at the same time. The 3NX builder
records the reference for runtime repair.

Public headers should mark the correct symbol type:

    extern bool PLUGIN_abcd_Function(u32 value);
    NEXUS_PLUGIN_EXTERNAL_FUNC(PLUGIN_abcd_Function);

    extern u32 PLUGIN_abcd_State;
    NEXUS_PLUGIN_EXTERNAL_OBJECT(PLUGIN_abcd_State);

For external function calls, keep the repaired address in the plugin's
import table and call through that pointer. Do not directly branch to
another plugin.


MENU API
--------
Rosalina plugins can use the public MENU API with:

    #include "sysplugin_menu.h"

and MENU in that plugin's allowed_refs.

You do not need MENU's implementation source to build against it.

Loader plugins cannot use the Rosalina MENU API because Loader and Rosalina
are separate plugin environments.


Transient .3on tools
--------------------
The dev kit already includes:

    sysplugin/make_transient_3on.py
    sysplugin/compress_3on_lzss.py
