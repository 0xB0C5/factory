$fa = 1;
$fs = 0.4;

thickness = 2;
wall_thickness = 2;
button_size = 12.5;
button_holder_thickness = 2;

switch_diameter = 6;
switch_x = -30;
switch_y = -25;
switch_height = 3.8;
switch_inset_diameter = 8;
switch_inset_depth = 1;


// Actual 9v battery dimensions:
// 48.5 mm × 26.5 mm × 17.5 mm
battery_length = 52; // extra space for connector.
battery_width = 27;
battery_depth = 18;

battery_holder_wall_thickness = 2;

case_width = 75;
case_height = 94;
corner_radius = 5;

viewport_width = 30.5;
viewport_height = 21.5;
viewport_y = -19.3;

lcd_width = 40;
lcd_height = 34;
lcd_depth = 6.3;
lcd_y = -22;


case_depth = thickness + button_holder_thickness + thickness + battery_depth + thickness;

screw_radius = 1.7;
screw_insert_radius = 2.2;
screw_head_radius = 3;
screw_depth = 12;

screw_x = case_width/2-corner_radius;
screw_y = case_height/2-corner_radius;

screw_positions = [
    [-screw_x, -screw_y],
    [-screw_x, screw_y],
    [screw_x, -screw_y],
    [screw_x, screw_y],
];

button_positions = [
    [-12.5, 8],
    [-12.5, 38],
    [-27.5, 23],
    [2.5, 23],
    [21.25, 33],
    [27.5, 13],
];

module button_cutout_2d() {
    square([button_size, button_size], center=true);
}

module button_holder() {
    holder_thickness = button_holder_thickness;
    translate([0, 0, thickness + holder_thickness/2]) cube([button_size+7.5, button_size-thickness, holder_thickness], center=true);
}

module switch_holder() {
    holder_radius = switch_diameter/2+2;
    linear_extrude(switch_inset_depth, scale=switch_diameter/switch_inset_diameter) {
        difference() {
            circle(holder_radius * switch_inset_diameter/switch_diameter);
            circle(switch_inset_diameter/2);
        }
    }
    translate([0, 0, switch_inset_depth]) linear_extrude(switch_height) {
        difference() {
            circle(holder_radius);
            circle(switch_diameter/2);
        }
    }
}

module screw_holder() {
    linear_extrude(case_depth-thickness) {
        difference() {
            circle(corner_radius);
            circle(screw_insert_radius);
        }
    }
    
    linear_extrude(case_depth-screw_depth) {
        circle(corner_radius);
    }
}

module lcd_holder() {
    bar_width = 3;
    bar_length = 8;
    hbar_x = lcd_width/2-bar_length/2+bar_width;
    hbar_y = lcd_height/2+bar_width/2;
    vbar_x = lcd_width/2+bar_width/2;
    vbar_y = lcd_height/2-bar_length/2+bar_width;
    linear_extrude(thickness + 2) {
        translate([hbar_x, hbar_y]) {
            square([bar_length, bar_width], center=true);
        }
        translate([hbar_x, -hbar_y]) {
            square([bar_length, bar_width], center=true);
        }
        translate([-hbar_x, hbar_y]) {
            square([bar_length, bar_width], center=true);
        }
        translate([-hbar_x, -hbar_y]) {
            square([bar_length, bar_width], center=true);
        }

        translate([vbar_x, vbar_y]) {
            square([bar_width, bar_length], center=true);
        }
        translate([vbar_x, -vbar_y]) {
            square([bar_width, bar_length], center=true);
        }
        translate([-vbar_x, vbar_y]) {
            square([bar_width, bar_length], center=true);
        }
        translate([-vbar_x, -vbar_y]) {
            square([bar_width, bar_length], center=true);
        }
    }
}

module case_shape_2d() {
    union() {
        square([case_width, case_height-2*corner_radius], center=true);
        square([case_width-2*corner_radius, case_height], center=true);
        
        for (screw_pos = screw_positions) {
            translate(screw_pos) {
                circle(corner_radius);
            }
        }
    }
}

module front_plate() {
    linear_extrude(thickness) difference() {
        case_shape_2d();
        
        translate([0, viewport_y]) {
            square([viewport_width, viewport_height], center=true);
        }
        
        translate([switch_x, switch_y]) {
            circle(switch_inset_diameter/2);
        }
        
        for (button_pos = button_positions) {
            translate(button_pos) {
                button_cutout_2d();
            }
        }
    }

    translate([switch_x, switch_y]) switch_holder();

    translate([0, lcd_y]) lcd_holder();

    intersection() {
        cube([case_width, case_height, 50], center=true);
        union() {
            for (button_pos = button_positions) {
                translate(button_pos) {
                    button_holder();
                }
            }
        }
    }
}


module front_walls() {
    difference() {
        union() linear_extrude(case_depth-thickness, convexity=3) {
            for (wall_sign = [-1, 1]) {
                translate([wall_sign*(case_width/2-wall_thickness/2), 0]) {
                    square([wall_thickness, case_height-2*corner_radius], center=true);
                }
                
                translate([0, wall_sign*(case_height/2-wall_thickness/2)]) {
                    square([case_width-2*corner_radius, wall_thickness], center=true);
                }
            }
        }
        
        charger_width = 11;
        charger_height = 6;
        charger_offset_z = 3.5;
        translate([-case_width/2+wall_thickness/2, case_height/2-corner_radius*2-thickness-battery_width/2-thickness, thickness+button_holder_thickness+thickness+charger_offset_z])
        cube([2*wall_thickness, charger_width, charger_height], center=true);
    }
}

module front() {
    front_plate();

    
    for (screw_pos = screw_positions) {
        translate(screw_pos) {
            screw_holder();
        }
    }
    front_walls();
}

module gear_shape_2d() {
    
    tooth_count = 20;
    r0 = 27;
    r1 = 30;
    difference() {
        polygon([
            for (i=[0:tooth_count-1])
            let (
                theta0 = i*360/tooth_count,
                theta1 = (i+0.5)*360/tooth_count
            )
            each [
                [r0*cos(theta0-1), r0*sin(theta0-1)],
                [r1*cos(theta0+1), r1*sin(theta0+1)],
                [r1*cos(theta1-1), r1*sin(theta1-1)],
                [r0*cos(theta1+1), r0*sin(theta1+1)],
            ]
        ]);
        circle(10);
        hole_count = 7;
        hole_dist = 18;
        for (i=[0:hole_count]) {
            theta = i*360/hole_count;
            translate([hole_dist*sin(theta), -hole_dist*cos(theta)]) circle(4);
        }
    }
}

module back() {

    linear_extrude(thickness, convexity=3) difference() {
        case_shape_2d();
        
        for (screw_pos = screw_positions) {
            translate(screw_pos) {
                circle(screw_radius);
            }
        }
    }
    
    translate([0, 0, 1]) color("green") {
        linear_extrude(thickness+0.4-1, convexity=3) {
            gear_shape_2d();
            
            edge_size = 5;
            
                
            for (sign_x = [-1,1]) for (sign_y = [-1,1]) {
                screw_pos = [sign_x*screw_x,sign_y*screw_y];
                
                translate([sign_x*(screw_x-corner_radius-edge_size/2), sign_y*(screw_y+corner_radius-edge_size/2)]) {
                    circle(edge_size/2);
                }
                
                translate([sign_x*(screw_x+corner_radius-edge_size/2), sign_y*(screw_y-corner_radius-edge_size/2)]) {
                    circle(edge_size/2);
                }
            }
            
            for (sign = [-1, 1]) {
                translate([0, sign*(screw_y+corner_radius-edge_size/2)]) square([case_width-4*corner_radius-edge_size, edge_size], center=true);
                
                translate([sign*(screw_x+corner_radius-edge_size/2), 0]) square([edge_size, case_height-4*corner_radius-edge_size], center=true);
            }
        }
        
    }
}


module battery_holder() {
    side_thickness = battery_holder_wall_thickness;

    holder_width = battery_length+1*side_thickness;
    holder_height = battery_width+2*battery_holder_wall_thickness;

    bar_depth = 0.75*battery_depth + thickness;
    gap_size = 10;

    linear_extrude(bar_depth) {
        translate([-holder_width/2 + side_thickness/2, 0]) {
            //square([side_thickness, holder_height], center=true);
        }

        translate([holder_width/2 - side_thickness/2, 0]) {
            square([side_thickness, holder_height], center=true);
        }

        translate([0, -holder_height/2 + side_thickness/2]) {
            square([holder_width/2, side_thickness], center=true);
        }
        translate([0, -holder_height/2 + side_thickness/2]) {
            square([holder_width, side_thickness], center=true);
        }

        translate([-gap_size/2, holder_height/2 - side_thickness/2]) {
            square([holder_width-gap_size, side_thickness], center=true);
        }
    }

    linear_extrude(thickness) {
        square([holder_width, holder_height], center=true);
    }
}

// front();

back();

// battery_holder();
