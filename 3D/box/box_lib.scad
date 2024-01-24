include <constants.scad>
use <shapes.scad>
$fn=60;
//box Library in den Ordner C:\Users\User\Documents\OpenSCAD\libraries\ kopieren
//------- Box  -------------
module Box()
{
color("Gainsboro")
difference() {
cuboid([x,y,z], fillet=b);

translate([0, 0, d])
cuboid([x-d,y-d,z+1], fillet=b);

translate([-y-y/2, -x-x/2, z/2-d])
cube(size = [x+300,y+300,d]);    
}
//Schraublöcher-----------
color("Ivory")
union() {
difference() {
translate([x/2-d/2-b+b/2, y/2-d/2-b+b/2, -z/2+d+0.01])
cylinder(h = z-(d*2), d=b+3);
translate([x/2-d/2-b+b/2, y/2-d/2-b+b/2, -z/2+d+0.01])
cylinder(h = z-(d+2), d=b);
}

difference() {
translate([-x/2+d/2+b-b/2, y/2-d/2-b+b/2, -z/2+d+0.01])
cylinder(h = z-(d*2), d=b+3);
translate([-x/2+d/2+b-b/2, y/2-d/2-b+b/2, -z/2+d+0.01])    
cylinder(h = z-(d+2), d=b);    
}

difference() {
translate([-x/2+d/2+b-b/2, -y/2+d/2+b-b/2, -z/2+d+0.01])    
cylinder(h = z-(d*2), d=b+3);
translate([-x/2+d/2+b-b/2, -y/2+d/2+b-b/2, -z/2+d+0.01])
cylinder(h = z-(d+2), d=b);
}


difference() {
translate([x/2-d/2-b+b/2, -y/2+d/2+b-b/2, -z/2+d])
cylinder(h = z-(d*2), d=b+3);
translate([x/2-d/2-b+b/2, -y/2+d/2+b-b/2, -z/2+d])
cylinder(h = z-(d+2), d=b);    
}
}
} 

//------- Deckel-------------
module Deckel(){
color("Silver")
difference() {

cuboid([x,y,z-(z-d1)], fillet=1, center=true);
    
translate([x/2-d/2-b+b/2, y/2-d/2-b+b/2, -3])
cylinder(h = d+d, d=b);
    
translate([-x/2+d/2+b-b/2, y/2-d/2-b+b/2, -3])
cylinder(h = d+d, d=b);    

translate([-x/2+d/2+b-b/2, -y/2+d/2+b-b/2, -3]) 
cylinder(h = d+d, d=b);

translate([x/2-d/2-b+b/2, -y/2+d/2+b-b/2, -3])
cylinder(h = d+d, d=b);    
}
}
//------------------------