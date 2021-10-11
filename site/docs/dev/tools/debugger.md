---
title: 'Skia Debugger'
linkTitle: 'Skia Debugger'
---

## Introduction

The Skia Debugger is a graphical tool used to step through and analyze the
contents of the Skia picture format. The tool is available online at
[https://debugger.skia.org](https://debugger.skia.org/) or can be run locally.

Features:

- Draw command and multiple frame playback
- Shows the current clip and matrix at any step
- Zoomed viewer with crosshair for selecting pixels.
- Breakpoints that stop playback when a pixel's color changes.
- GPU or CPU backed execution.
- GPU op bounds visualization
- Android offscreen layer visualization
- Shared resource viewer

<img src="../onlinedebugger.png" style="display: inline-block;" />

## User Guide

SKP files can contain a single frame or multiple frames. Single frame files have
the .skp extension and Multi-frame files have the .mskp extension. In the online
debugger linked above, Open a [sample mskp file](/docs/dev/tools/calendar.mskp)
or capture one from an android device using the
[instructions here](https://sites.google.com/a/google.com/skia/android/skp-from-framework).

### Command Playback and Filters

Try playing back the commands within the current frame using the lower play
button <img src="../playcommands.png" style="display: inline-block;" />, (the
one not in a circle) You should see the image built up one draw at a time.

Many commands manipulate the matrix or clip but don't make any visible change
when run. Try filtering these out by pasting
`!drawannotation save restore concat setmatrix cliprect` in to the filter
textbox just below the command playback controls. Press enter to apply the
filter, and resume playback if it was paused. This will have the effect of
making the playback appear to be much faster as there are only 29 commands in
the first frame of the sample file that pass this filter.

Try pausing command playback and stepping forward and back through the commands
using `,` (comma) and `.` (period).

> Filters are case insensitive, and the only supported logical operator is !
> (not) which applies to the entire filter and is only recognised when it occurs
> at the beginning.

Any command can be expanded using the
<img src="../expand.png" style="display: inline-block;" /> icon to see all of
the parameters that were recorded with that command.

Commands can be disabled or enabled with the checkbox that becomes available
after expanding the command's detail view.

Jog the command playhead to the end of the list with the
<img src="../end.png" style="display: inline-block;" /> button.

### Frame playback

<img src="../frameplayback.png" style="display: inline-block;" />

The sample file contains multiple frames. Use the encircled play button to play
back the frames. The current frame is indictated by the slider position, and the
slider can be set manually. Frames can be stepped through with `w` (back) and
`s` forward. `p` pauses or unpauses the frame playback.

Not all frames in a file will have the same number of commands. When the command
playhead is left at the end of the list the debugger will play every frame to
the end of its list. If the command playhead is somewhere in the middle, say
155, the debugger will try to play every frame to its 155th command.

### Resources Tab

<img src="../resources.png" style="display: inline-block;" />

Any resources that were referenced by commands in the file appear here. As of
Dec 2019, this only shows images.

Any resource can be selected and viewed. You may find it helpful to toggle the
Light/Dark setting if you cannot see an image.

Images' names are formatted as `7 @24205864 (99, 99)` where `7` is the index of
the image as it was saved in the file, `@24205864` is it's address in wasm
memory, for cross referencing with DrawImage\* commands in the command list
which also show this address, and `(99, 99)` is the size of the image.

The resource viewer allows a user to determine if an image was not effectively
shared between frames or draw commands. If it occurs more than once in the
resource tab, then there were multiple copies of it with different generation
ids in the process that recorded the SKP.

### Android Layers

<img src="../layers.png" style="display: inline-block;" />

When MSKPs are recorded in Android, Extra information about offscreen hardware
layers is recorded. The sample google calendar mskp linked above contains this
information. You will find two layers on frame 3.

There are two kinds of events relevant to recorded android layer use.

1. Draw Events - points when an offscreen surface was drawn to. They may be
   complete, meaning the clip equalled the surface's size, or partial, meaning
   the clip was smaller.
2. Use events - points when the offscreen surface was used as an SkImage in the
   main surface.

Layers are shown as boxes in the bottom right of the interface when viewing a
frame where a layer draw event occurred. Each Layer box has two buttons:
`Show Use` will cycle through use events for that layer in the current frame if
there are any, and `Inspector` will open the draw event as if it were a single
frame SKP. you can play back it's commands, enable or disabled them, inspect GPU
op bounds or anything else you could do with an ordinary SKP. Exit the inspector
by clicking the `Exit` button on the layer box.

### Crosshair and Breakpoints

<img src="../crosshair.png" style="display: inline-block;" />

Clicking any point in the main view will toggle a red crosshair for selecting
pixels. the selected pixel's color is shown in several formats on the right
pane. A zoomed view centered on the selected pixel is shown below it. The
position can be moved precicely by either clicking neighboring pixels in the
zoom view, or using `H` (left) `L` (right) `J` (down) `K` (up).

When "Break on change" is selected, command playback will pause on any command
which changes the color of the selected pixel. this can be used to find the
command that draws something you see in the viewer.

### GPU Op Bounds and Other settings

<img src="../settings.png" style="display: inline-block;" />

Each of the filtered commands from above has a colored number to its right
<img src="../gpuop.png" style="display: inline-block;" />. This is the GPU
operation id. When multiple commands share a GPU op id, this indicates that they
were batched together when sent to the GPU. In the WASM debugger, this goes
though WebGL.

There is a "Display GPU Op Bounds" toggle in the upper right of the interface.
Turning this on will show a colored rectangle to represent the bounds of the GPU
op of the currently selected command.

GPU - Controls which backend Skia uses to draw to the screen. GPU in the online
wasm debugger means WebGL. CPU means skia draws into a surface in memory which
is copied into an HTML canvas without using the GPU.

Light/Dark - this toggle changes the appearance of the checkerboard behind the
main view and zoom views to assist in viewing content with transparency.

Display Overdraw Viz - This vizualization shows a red overlay that is darker in
propertion to how much overdraw has occurred on a pixel. Overdraw meaning that
the pixel was drawn to more than once.

- As of Dec 2019, this feature may not be working correctly.

### Image fit and download buttons.

<img src="../settings.png" style="display: inline-block;" />

These buttons resize the main view. they are, from left to right:

Original size - the natural size of the canvas, when it was recorded. Fit to
page - shrink enough that the whole canvas fits in the center pane. Fit to page
width - make the canvas fit horizontally but allow scrolling vertically Fit to
page height - make the canvas fit vertically but allow scrolling horizontally.

next to these is a 5th, unrelated download button which saves the current canvas
as a PNG file.

## Building and running locally

Begin by following the instructions to
[download and build Skia](/docs/user/build). Next, you'll need Skia's infrastructure repository,
which can be downloaded with

<!--?prettify lang=sh?-->
    git clone https://skia.googlesource.com/buildbot

See further instructions in buildbot/debugger-app/README.md.
