# 3NX SysPlugins

3NX SysPlugins add dynamically loaded plugins to Nexus3DS.

Plugins use the `.3nx` format and can target either **Loader** or **Rosalina**. Multiple plugins can be loaded together, ordered by priority, and may optionally reference or depend on other plugins.

Unlike traditional fixed-address patches, 3NX plugins are relocatable. Nexus3DS allocates memory for them at runtime and repairs their host references using the matching `3NR` environment data.

## Features

- Relocatable `.3nx` plugins
- Loader and Rosalina plugin support
- Multiple simultaneously loaded plugins
- Numeric priority and plugin stacking
- Cross-plugin references and dependencies
- Runtime host-symbol repair
- Semantic `PLG_MARKER` references for internal host code locations
- Automatic plugin failure and dependency handling

## Plugin Development

Unpack this repository over Nexus3DS (currently clone [my fork](https://github.com/Blurro/Nexus3DS)) to create the plugin dev environment.

Plugin code and data are placed into dedicated sections before being extracted and packaged into a `.3nx`.

Each plugin has a unique four-character ID within its target module.

Build using `./makeplugin.sh`. In the plugin dev environment, `make` is disabled.

[See examples here!](https://github.com/Blurro/Nexus3DS-SysPlugins)

## Installation

Place built plugins in:

```text
/luma/plugins/
```

Plugins are selected and loaded automatically during boot. Make sure that 'load external firms/modules' is enabled in SELECT settings.

The active `boot.firm` file must have its matching `boot.3nr` repair data available so host references inside installed plugins can be resolved.
This is renamed to something identifiable allowing fastboot firm switching compatibility.

## Status

3NX SysPlugins are currently an experimental extension to Nexus3DS and are under active development.
