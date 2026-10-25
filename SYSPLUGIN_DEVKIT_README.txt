Nexus3DS Sysplugin Dev Kit
==========================

Base workflow
-------------
Apply this ZIP over a clean, unbuilt, Nexus3DS clone.
(Currently the /dev/ branch is the latest)
Then apply your plugin overlay.

Edit only the CONFIG block at the top of makeplugin.sh, then run:

    ./makeplugin.sh


Plugin overlays
---------------
Files listed in SYSPLUGIN_DEVKIT_OWNED_FILES.txt belong to the dev kit and
generally should not be replaced by plugin overlays.

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
Rosalina and Loader plugins can use a sysmodule-respective public MENU API with:

    #include "sysplugin_menu.h"

and MENU in that plugin's allowed_refs.

You do not need MENU's source to build against it.


.3on tools
----------
An entry in either ROSALINA_PLUGIN_CONFIG or LOADER_PLUGIN_CONFIG can be a
transient. Set its references field to "transient" alone:

    "demo|mytransient||transient"

The priority field can be left blank or set to a number like 999. It is
ignored for transients, so either way the output is mytransient.3on. A
transient can be loaded by Loader or Rosalina when there is code to load it.

LZ10 compression
----------------
Use sysplugin/compress_lzss.py to compress any nonempty file, including
assets you plan to put in plugin metadata:

    python3 sysplugin/compress_lzss.py image.bin image.bin.lz

The output is a raw LZ10 stream with a 0x10 header and a 24-bit original
size. Inputs can be up to 16,777,215 bytes. MENU's
PLUGIN_MENU_UnpackLz10File can unpack a range containing that stream from an
open plugin file to an SD file. Pass it the offset and size of the compressed
data in the plugin file.

The compressor checks that its output decompresses back to the original
file. Compression may make a file larger if it has little repeated data.
