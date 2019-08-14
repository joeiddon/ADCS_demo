These are the OpenSCAD source files (code) that are used to generate diffrent 3D frame designs for the demo.

In the `stls` folder in the parent directory, you can find the `.stl` 3D model files exported from OpenSCAD using this source.

All files prefixed with `frame_` contain different designs for the demo.

The first design, `frame_freestanding.scad`, can be suspended as a standalone demo; hence it does not fit the BeeKit.

The later designs, which fit the BeeKit, are `frame_cutouts.scad` and `frame_edged.scad`. These are literally identical files, bar the `wheel_cutouts_diameter` variable. In the `frame_cutouts.scad` file this is set to be larger than `100mm` so that there are cutouts in the walls of the BeeKit module. Conersely, the `frame_edged.scad` has a value less than `100mm` so that the BeeKit interface has full walls all the way around - so it is "edged". If this is not clear, see the renders of their corresponding `.stl` files here in GitHub.

I only ever 3D printed and tested the `frame_cutouts.scad` design since I felt it served better as a demonstration. I leave the `frame_edged.scad` design here in case someone wishes to modify this in the future.

Finally, the `beekit_interface.scad` file contains a single module: `beekit_interface(layers)`. This returns a solid structure that spans "layers" many mounting holes in the BeeKit frame. Both `frame_edged.scad` and `frame_cutouts.scad` use this. Again, this code may come in useful for a future developer working in OpenSCAD.
