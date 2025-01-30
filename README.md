# Wave Function Collapse (Godot Addon)

This addon is the result of my Final Degree Project in Computer Engineering at Universitat Politècnica de Catalunya, where I aimed to faithfully adapt and implement the [original version](https://github.com/mxgmn/WaveFunctionCollapse) of the Wave Function Collapse (WFC) algorithm created by [Maxim Gumin](https://github.com/mxgmn). While the addon is functional, it still lacks some features and quality-of-life improvements that couldn't be implemented due to time constraints.

## Summary

This plugin adds the WFCGenerator2D node to Godot 4.3, enabling customization of the WFC algorithm execution and allowing association with TileMapLayers to learn from example maps and generate locally similar tile-based outputs.

## Features

- Faithful adaptation of the original algorithm, replacing bitmaps with tile maps. To replicate the original behavior, you can use single-color tiles and adjust relevant properties.
- Original overlapping model implementation
- TileMapLayer integration. Pattern rotations and reflections are managed through customizable TileSet properties
- Backtracking support. The current version allows reverting to previous states and configuring backtracking parameters. Planned improvements for v1.0 include backjumping support and enhanced backtracking mechanics

## Usage

First, create a TileMapLayer node that will serve as the algorithm's input example:

1. Add the WFCGenerator2D node to your scene

<p align="center"><img alt="Usage Step 1" src="media/readme/Usage step 1.gif"></p>

2. Configure the WFCGenerator2D node properties to define algorithm behavior

<p align="center"><img alt="Usage Step 2" src="media/readme/Usage step 2.gif"></p>

3. Assign TileMapLayer nodes (input, output, and optional pattern display) to the WFCGenerator2D node

<p align="center"><img alt="Usage Step 3" src="media/readme/Usage step 3.gif"></p>

4. Define custom properties for each tile in the TileSet

<p align="center"><img alt="Usage Step 4" src="media/readme/Usage step 4.gif"></p>

For included demo examples:

5. Run the scene. The first button extracts and displays patterns used for generation

<p align="center"><img alt="Usage Step 5" src="media/readme/Usage step 5.gif"></p>

6. The second button generates the output map

<p align="center"><img alt="Usage Step 6" src="media/readme/Usage step 6.gif"></p>

## Installation

Available on the [Godot Asset Library](https://godotengine.org/asset-library/asset) - install directly through Godot's Addon Manager (remember to enable the plugin in project settings). Alternatively, download the latest release from our [releases page](https://github.com/Rayogenesis/WaveFunctionCollapse-GodotAddon/releases).

## Credits

- [Flowers](https://github.com/mxgmn/WaveFunctionCollapse/blob/master/samples/Flowers.png) and [Rooms](https://github.com/mxgmn/WaveFunctionCollapse/blob/master/samples/Rooms.png) tile sets replicate original examples by [Maxim Gumin](https://github.com/mxgmn)
- [Tiny Dungeon](https://kenney.nl/assets/tiny-dungeon) and [Tiny Battle](https://kenney.nl/assets/tiny-battle) assets by [Kenney](https://kenney.nl/)