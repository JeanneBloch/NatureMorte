e=0.01;
$fn=64;

inner_d = 23.5;
thickness = 5;
floor = 1;

union() {
    difference() {
        hull() {
            cylinder(h=thickness, d=inner_d+3);
            
            translate([-thickness/2, -inner_d, 2])
            cube([thickness, inner_d/2, thickness-2]);
        }
        
        translate([0, 0, floor])
        hull() {
            cylinder(h=thickness, d=inner_d);

            translate([-0.6*thickness/2, -inner_d-e, 2])
            cube([0.6*thickness, inner_d/2, thickness-2]);
        }
    }
    
    translate([0, 0, floor + e])
    difference() {
        cylinder(h=2, d=inner_d);
        
        translate([0, 0, -e])
        cylinder(h=5+2*e, d=inner_d-2);
        
        w = 0.8*inner_d;
        translate([-w/2, -25, -e])
        cube([w, 20, 15]);
    }
}

translate([0, 0, e])
for(a=[-1,1]) {
    h = 0.4 * thickness;
    l = 0.8*inner_d;
    th = 1;

    scale([a,1,1])
    translate([0.3*inner_d, -l/2, 0])
    rotate([0,180,0])
    difference() {
        cube([2*th, l, h]);
        
        translate([-e, th-e, -e])
        cube([10, l-2*th-2*e, h-th]);
    }
}