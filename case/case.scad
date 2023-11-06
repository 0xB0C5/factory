$fa = 1;
$fs = 0.4;

thickness = 2;
wall_thickness = 3;
button_size = 12.5;

switch_diameter = 6;
switch_x = -33;
switch_y = -25;
switch_height = 3.8;
switch_inset_diameter = 8;
switch_inset_depth = 2;


// Actual 9v battery dimensions:
// 48.5 mm × 26.5 mm × 17.5 mm
battery_length = 52; // extra space for connector.
battery_width = 27;
battery_depth = 18;

battery_holder_wall_thickness = 2;

case_width = 90;
case_height = 96;
corner_radius = 5;

battery_x = case_width/2 - battery_length/2 - wall_thickness - battery_holder_wall_thickness;
battery_y = -case_height/2 + battery_width/2 + 2*corner_radius + battery_holder_wall_thickness;

viewport_width = 30.5;
viewport_height = 21.5;
viewport_y = -20.3;

lcd_width = 40;
lcd_height = 34;
lcd_depth = 6.3;
lcd_y = -23;


case_depth = thickness + lcd_depth + thickness + battery_depth + thickness + 1;

screw_radius = 1.7;
screw_insert_radius = 2;
screw_head_radius = 3;
screw_depth = 8;

screw_x = case_width/2-corner_radius;
screw_y = case_height/2-corner_radius;

screw_positions = [
    [-screw_x, -screw_y],
    [-screw_x, screw_y],
    [screw_x, -screw_y],
    [screw_x, screw_y],
];

button_positions = [
    [-20, 8],
    [-20, 38],
    [-35, 23],
    [-5, 23],
    [20, 28],
    [35, 13],
];

module button_cutout_2d() {
    square([button_size, button_size], center=true);
}

module button_holder() {
    holder_thickness = 2;
    translate([0, 0, thickness + holder_thickness/2]) cube([button_size+7.5, button_size-thickness, holder_thickness], center=true);
}

module switch_holder() {
    holer_radius = switch_diameter/2+3;
    linear_extrude(switch_inset_depth, scale=switch_diameter/switch_inset_diameter) {
        difference() {
            circle(holer_radius * switch_inset_diameter/switch_diameter);
            circle(switch_inset_diameter/2);
        }
    }
    translate([0, 0, switch_inset_depth]) linear_extrude(switch_height) {
        difference() {
            circle(holer_radius);
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

    for (button_pos = button_positions) {
        translate(button_pos) {
            button_holder();
        }
    }
    
    translate([32, battery_y]) {
        linear_extrude(thickness + lcd_depth) {
            square([15, 20], center=true);
        }
    }
}


module front_walls() {
    linear_extrude(case_depth-thickness) {
        for (wall_sign = [-1, 1]) {
            translate([wall_sign*(case_width/2-wall_thickness/2), 0]) {
                square([wall_thickness, case_height-2*corner_radius], center=true);
            }
            
            translate([0, wall_sign*(case_height/2-wall_thickness/2)]) {
                square([case_width-2*corner_radius, wall_thickness], center=true);
            }
        }
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

module back() {
    linear_extrude(thickness) difference() {
        case_shape_2d();
        
        for (screw_pos = screw_positions) {
            translate(screw_pos) {
                circle(screw_radius);
            }
        }
    }
    
    /*
    linear_extrude(2.5) difference() {
        case_shape_2d();
        
        for (screw_pos = screw_positions) {
            translate(screw_pos) {
                circle(screw_head_radius);
            }
        }
    }
    */
}


module battery_holder() {
    side_thickness = battery_holder_wall_thickness;

    holder_width = battery_length+2*side_thickness;
    holder_height = battery_width+2*battery_holder_wall_thickness;
    
    bar_depth = battery_depth + thickness;
    gap_size = 10;
    
    linear_extrude(bar_depth) {
        translate([-holder_width/2 + side_thickness/2, 0]) {
            square([side_thickness, holder_height], center=true);
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

//back();

battery_holder();

/*
translate([battery_x, battery_y, lcd_depth+thickness]) {
    // color("red") translate([0, 0, battery_depth/2+thickness]) cube([battery_length, battery_width, battery_depth], center=true);
    
    color("green") battery_holder();
}
*/