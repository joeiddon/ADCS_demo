/*
Design by Joe Iddon.
*/


/*** PRINT DESIGN UPSIDE DOWN TO AVOID USING SUPPORTS ***/

use <beekit_interface.scad> //need the `beekit_interface(layers)` function

$fn = 30;

//parameters for ADCS system plate (so motor mounting and LDRs etc)
width = 100; //square, so length = width
thickness = 2.4; //thickness of plate
motor_ring_thickness = 1; //thickness of larger motor cutout ring
motor_ring_diameter = 40.5; //larger motor cutout ring diameter
motor_diameter = 38; //diameter of smaller motor cutout ring
screw_hole_diameter = 3.75; //diameter of holes for motor mounting
screw_hole_dist = 23.65; //distance of motor holes from center
ldr_hole_diameter = 1.7; //diameter of holes for LDR leads
ldr_hole_width = 3.3; //shorter distance between LDR lead holes
ldr_hole_length = 5; //longer distance between LDR lead holes
ldr_hole_dist = 38; //distance of close LDR holes from center

//parameters for connection of ADCS system plate with beekit interface
wheel_cutout_diameter = 110; //diameter of cutout for momentum wheel (HDD disk)
total_height = 12; //total height of the frame (2*buffer_height in beekit_interface)

intersection(){
    beekit_interface(1);
    difference(){
        union(){
            translate([0,0,total_height/2-thickness])
            ADCS_plate();
            translate([-width/2,-width/2,-total_height/2])
            cube([width,width,total_height-thickness]);
        }
        translate([0,0,-total_height/2])
        cylinder(d=wheel_cutout_diameter,h=total_height-thickness,$fn=60);
    }
}

module ADCS_plate(){
    difference(){
        translate([0,0,thickness/2])
        cube([width,width,thickness],center=true);
        cylinder(d=motor_diameter,h=thickness,$fn=60);
        //translate([0,0,thickness-motor_ring_thickness])
        cylinder(d=motor_ring_diameter,h=motor_ring_thickness,$fn=60);
        for (angle = [0,120,240]){
            rotate(angle+45)
            translate([screw_hole_dist, 0, 0])
            cylinder(d=screw_hole_diameter,h=thickness);
        }
        for (angle = [0,90]){
            rotate(angle)
            translate([ldr_hole_dist, 0, 0])
            ldr_holes();
        }
    }
}

module ldr_holes(){
    translate([0,-ldr_hole_width/2,0])
    cylinder(d=ldr_hole_diameter,h=thickness);
    translate([0, ldr_hole_width/2,0])
    cylinder(d=ldr_hole_diameter,h=thickness);
    translate([ldr_hole_length,-ldr_hole_width/2,0])
    cylinder(d=ldr_hole_diameter,h=thickness);
    translate([ldr_hole_length, ldr_hole_width/2,0])
    cylinder(d=ldr_hole_diameter,h=thickness);
}
