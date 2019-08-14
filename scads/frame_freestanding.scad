/*
Design by Joe Iddon.
*/

$fn = 30;
width = 100;
thickness = 2.4;
motor_ring_thickness = 1;
motor_ring_diameter = 40.5;
motor_diameter = 38;
screw_hole_diameter = 3.75;
screw_hole_dist = 23.65;
ldr_hole_diameter = 1.7;
ldr_hole_width = 3.3;
ldr_hole_length = 5;
ldr_hole_dist = 55;

difference(){
    translate([0,0,thickness/2])
    cube([width,width,thickness],center=true);
    cylinder(d=motor_diameter,h=thickness,$fn=60);
    translate([0,0,thickness-motor_ring_thickness])
    cylinder(d=motor_ring_diameter,h=motor_ring_thickness,$fn=60);
    for (angle = [0,120,240]){
        rotate(angle)
        translate([screw_hole_dist, 0, 0])
        cylinder(d=screw_hole_diameter,h=thickness);
    }
    for (angle = [-45, 45]){
        rotate(angle)
        translate([ldr_hole_dist, 0, 0])
        ldr_holes();
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