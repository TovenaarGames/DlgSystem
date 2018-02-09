# DlgSystem (Dialogue System) Plugin

Master branch Unreal version: 4.18

## Features:

- 100% free, source code included
- Full Blueprint and C++ support
- Events and conditions
- Branching based on different node types and conditions
- Well structured runtime and editor modules
- Full editor support including:
    - Graph Editor
    - Search window
    - Dialogue Browser to get a good overview of all the dialogues
    - Customized blueprint nodes to handle certain events and conditions in Blueprint easily
    - Settings panel in Editor Preferences to modify the layout
    - Option to import from and export to plain text formats (e.g. json)

Not yet convinced? Have you seen our [`showcase page`](https://gitlab.com/NotYetGames/DlgSystem/wikis/Feature-Showcase)?

## General Information

See the [Wiki](https://gitlab.com/NotYetGames/DlgSystem/wikis/home) for the manual and tutorials.

[DlgSystemExample project](https://gitlab.com/NotYetGames/DlgSystemExample) is also provided.

This plugin is primary developed and used for our own [game](http://www.warriorb.com).
We have put significant effort into it and decided to make it open source in order to help out other teams facing similar challenges.
Contribution, feedback and constructive criticism are appreciated and we would be really happy to hear about your game using our tool. 
Should it prove useful to you, the best way to say thanks is to mention us and our game to your own audience.

Brought to you by Not Yet:
* Discord: https://discord.gg/5gH5tWR
* Website: [notyet.eu](http://www.notyet.eu)
* Twitter: https://twitter.com/NotYetDevs

## Conventions

### Branches
The supported unreal versions have their own branches.
The master branch is usually updated to the newest engine versions as soon as possible (way before the engine version is in a stable state).
The outdated version branches are kept but not updated anymore.

### Code Style

The code style follows the one from the [UE Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/)
with the additional changes located in the [`.editorconfig`](.editorconfig).

### Commit messages

Every commit message that has `[C]` means that after updating to that commit, a recompile is required.
Eg: `[C] Added awesome Dialogue feature`

Every commit message that has `[G]` means that after updating to that commit the solution files should be regenerated (and recompiled, *obvious*).
(`Right click on project file` -> `Generate Visual Studio project files`) (or from the command line regenerate the project).

Eg: `[G] New Dialogue Editor mode`