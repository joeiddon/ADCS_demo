/*
Design by Joe Iddon.
*/

$fn = 30;

module beekit_interface(layers){
    //using dimensions in https://open-cosmos.com/beekit-usermanual
    //there is a dimension missing from that schematic though:
    //the width of every side with cutouts is 82.6 - this was confirmed
    //from an autodesk 360 model
    //x-axis points in same direciton as two of the fasteners
    //layers determines the height of this mechanical interface
    //module - e.g. a value of `2` means a height of (2*12+12)mm
    //so would have a potential of 16 holes to be screwed in to the
    //beekit frame
    width = 100; //square, so length = width
    cutout_width = 82.6; //width of all the "flat" sides (after cutouts)
    //meaning of all indent measurements seen from schematic
    x_indent = 95.8;
    y_indent_close = 95.6;
    y_indent_far = 96;
    hole_sep = 12; //vertical separation between screw holes
    buffer_height = 6; // (12 / 2) - so doesn't end on a screw hole
    hole_diameter = 2.1; //needs experimentation if 3d printing
    hole_dist = 2.55; //distance of screw holes from nearest edges
    hole_depth = 7; //random choice, schematic defined must be > 5
    
    height = (layers-1)*hole_sep+buffer_height*2;
    union(){
        cube([width,cutout_width,height], center=true);
        cube([cutout_width,width,height], center=true);
        translate([-x_indent/4,0,0])
        difference(){ //hole block facing the same way ("close")
            cube([x_indent/2,y_indent_close,height], center=true);
            for (hole = [0:layers-1])
            translate ([0,0,-height/2+buffer_height+hole*hole_sep]){
                translate([-x_indent/4,y_indent_close/2-hole_dist,0])
                rotate([0,90,0]) cylinder(d=hole_diameter,h=hole_depth);
                translate([-x_indent/4,-(y_indent_close/2-hole_dist),0])
                rotate([0,90,0]) cylinder(d=hole_diameter,h=hole_depth);
            }
        }
        translate([x_indent/4,0,0])
        difference(){ //hole block facing outwars ("far")
            cube([x_indent/2,y_indent_far,height], center=true);
            for (hole = [0:layers-1])
            translate([0,0,-height/2+buffer_height+hole*hole_sep]){
                translate([x_indent/4-hole_dist,y_indent_far/2,0])
                rotate([90,0,0]) cylinder(d=hole_diameter,h=hole_depth);
                translate([x_indent/4-hole_dist,-y_indent_far/2,0])
                rotate([-90,0,0]) cylinder(d=hole_diameter,h=hole_depth);
            }
        }
    }
}

beekit_interface(2);