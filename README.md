# 3NX Sysplugins

<img width="341" height="174" alt="image" src="https://github.com/user-attachments/assets/8e180e74-a278-4d4c-97da-251bceee5a2a" />

.3NX Sysplugins (**3**DS **N**exus e**X**tension) add dynamically loaded plugins to [Nexus3DS](https://github.com/2b-zipper/Nexus3DS/tree/dev), allowing new features to be added to Loader or Rosalina without having to build everything directly into `boot.firm`.

Plugins are relocatable, can be loaded together, reference each other, even survive updates by repairing themselves against one another and Nexus3DS.

Check out my **[Sysplugin Showcase Presentation](https://blurro.github.io/sysplugins/Sysplugin%20Showcase.pdf)**

## Features

* Relocatable `.3nx` plugins
* Loader and Rosalina plugin support
* Multiple plugins loaded together, ordered by priority
* Cross-plugin references and dependencies
* Runtime host-symbol repair using matching `3NR` data
* `PLG_MARKER` support for referencing internal Nexus3DS code without relying on fixed source locations
* Plugin metadata packing
* Multiple plugins can be stacked into a single `.3nx`
* Automatic rejection/unloading when plugins or their dependencies fail
* Tools for creating transient `.3on` plugins

## Using Sysplugins

Place `.3nx` files in:

```text
/luma/plugins/
```

Make sure **Load external FIRMs and modules** is enabled in the SELECT boot configuration.

Plugins are selected automatically during boot. Lower priority numbers run first, with the filename and stack order used to resolve ties.

Your active `boot.firm` must also have its matching `3NR` repair data available. Nexus3DS identifies this data for the current environment, allowing different compatible `boot.firm` builds to coexist when using things such as fastboot3DS.

[See my current Sysplugins here!](https://github.com/Blurro/Nexus3DS-Sysplugins)

## Plugin Development

The dev kit is designed to be placed over a **clean, unbuilt, sysplugin-capable Nexus3DS source tree**.

Then place your plugin's source/assets over that and configure the `CONFIG` block near the top of `makeplugin.sh`.

A plugin entry looks like:

```bash
ROSALINA_PLUGIN_CONFIG=(
    "blur|blurPLGbase|10|"
)

LOADER_PLUGIN_CONFIG=(
    "coin|coinloader|50|"
)
```

Each entry contains:

```text
id|output_name|priority|allowed_refs
```

* **id** - Unique 4-character plugin ID for that module
* **output_name** - Name of the generated plugin
* **priority** - Determines plugin/Main() order, lower runs first
* **allowed_refs** - Other plugin IDs this plugin is allowed to reference

Then just run:

```bash
./makeplugin.sh
```

No normal top-level `make` is needed. In fact, the dev environment deliberately disables it so you don't accidentally build `boot.firm` instead of your plugin.

Generated plugins are written beside `makeplugin.sh`, for example:

```text
blurPLGbase.10.3nx
coinloader.50.3nx
```

Some plugin projects may also include their own `pre_makeplugin.sh` for preparing assets before the normal build.

<details>
<summary>Dev kit / plugin source rules</summary>

The dev kit owns its generic build infrastructure.

Files listed in:

```text
SYSPLUGIN_DEVKIT_OWNED_FILES.txt
```

should **not** be copied or modified by individual plugin projects.

The one intentional exception is the delimited `CONFIG` block inside `makeplugin.sh`.

A plugin overlay should generally only contain:

* Its own source
* Its own headers
* Its own assets
* Plugin-specific preparation scripts if needed
* Its configuration changes to `makeplugin.sh`

This means plugins don't each carry around slightly different copies of the linker scripts, 3NX generator or shared headers and then mysteriously explode six months later when one copy becomes outdated.

The dev kit also provides the standard section macros:

```c
PLUGIN_MAIN(id)
PLUGIN_CODE(id)
PLUGIN_RODATA(id)
PLUGIN_DATA(id)
PLUGIN_BSS(id)
```

through:

```c
#include "sysplugin_plugin.h"
```

</details>

<details>
<summary>Cross-plugin references</summary>

Plugins can export symbols using their 4-character ID:

```c
PLUGIN_abcd_Function
PLUGIN_abcd_SomeObject
```

A plugin using them adds `abcd` to its `allowed_refs`.

The provider does **not** have to be compiled in the same build. The reference is stored in the `.3nx` and repaired at runtime when the required provider is available.

For example:

```bash
ROSALINA_PLUGIN_CONFIG=(
    "abcd|provider|10|"
    "test|consumer|20|abcd"
)
```

Because `abcd` runs before `test`, it acts as a dependency. If the provider is rejected or unloads before the consumer runs, the dependent plugin is rejected too.

Public cross-plugin functions and objects should use the declarations/macros supplied by `sysplugin_symbols.h`, and relocatable calls should go through a repaired plugin table rather than directly branching to another plugin.

Rosalina plugins can similarly reference the public MENU API through:

```c
#include "sysplugin_menu.h"
```

and by adding `MENU` to their allowed references.

Loader and Rosalina are separate plugin environments, so Loader plugins cannot reference Rosalina MENU.

</details>

<details>
<summary>PLG_MARKER and host references</summary>

A plugin can reference locations inside Nexus3DS itself without hardcoding an address.

`PLG_MARKER` is placed after an existing semantic piece of host code:

```c
existing_code(); // PLG_MARKER(example_marker)
```

The dev kit uses GCC and DWARF information to identify that construct and records its linked address in the matching `3NR`.

Plugin code can then have that reference repaired when it loads.

This makes normal formatting and source movement much less likely to break a plugin compared to traditional fixed-address patches.

The marker comments belong to **Nexus3DS host code**, not plugin-owned code.

More detailed marker examples and rules are documented directly inside `makeplugin.sh`.

</details>

<details>
<summary>Metadata and stacked plugins</summary>

Extra files can be appended as metadata to a plugin through `METADATA_CONFIG`.

For example:

```bash
METADATA_CONFIG=(
    "coinrosalina|metadata.bin|icon.bin"
)
```

Multiple completed plugins can also be combined into one `.3nx`:

```bash
STACKED_PLUGIN_CONFIG=(
    "Playcoinz|10|coinloader,coinrosalina"
)
```

The individual plugins still retain their own module, ID and execution ordering. Stacking just allows them to be distributed as one file.

</details>

## Dev Kit Notes

The first plugin build automatically generates the Nexus3DS sysplugin entry header if it does not already exist. Temporary build state used for this is cleaned up automatically.

`makeplugin.sh` also handles incremental builds, semantic marker generation, relinking when needed, `.3nx` creation, metadata and stacking. You should not need to manually invoke the individual generator scripts during normal plugin development.

3NX Sysplugins and their development tools are still under active development, so the format/API may continue to evolve.
