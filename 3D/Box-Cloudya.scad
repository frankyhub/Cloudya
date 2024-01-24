//               Box und Deckel 17.5.2022 khf
//                           Cloudya

//************************   Auswahl   *****************************
//                   1:Box  2:Deckel 

                           part = "1";

//******************************************************************


//************************   Parmeter   *****************************
x = 80;  //Breite
y = 100;  //Länge
z = 50;  //Höhe incl Deckel
d1 = 4;   //Wand-Dicke mind. 3mm
b1 = 3;   //Durchmesser Bohrung mind. 3mm
//******************************************************************


a=3;
d= d1<a ? 3 : d1;

c=3;
b= b1<c ? 3 : b1;



print_part();



module print_part() 
 {
	if (part == "1") { 
        
        difference() {
		Box();
                                    //Durchbruch Display Wetterstation V2
            translate(v = [36, 0, 2])
            cube(size = [10,35,19.5], center=true);
            
                                    //Durchbruch 9V
            translate(v = [17, -50, -8.2])
            cube(size = [11,11,11], center=true);
            
                                    //Durchbruch USB
            translate(v = [-10, -50, -11.7])
            cube(size = [15,11,8], center=true);
            
                                                //Durchbruch Siebesegment-Anzeige
            rotate([90, 90, 0]) {         
            translate(v = [-1, -38, 0])
            cube(size = [21.2,21,50.7], center=true);
            }
            

         } 
  	} 
        
    else if (part == "2") {
        difference() {		
        Deckel();
        
 //Senklöcher                   
translate([36.5, 46.5, 3])
cylinder(h = 7, r1 = 1, r2 = 5, center = true);

translate([-36.5, 46.5, 3])
cylinder(h = 7, r1 = 1, r2 = 5, center = true);

translate([36.5, -46.5, 3])
cylinder(h = 7, r1 = 1, r2 = 5, center = true);

translate([-36.5, -46.5, 3])
cylinder(h = 7, r1 = 1, r2 = 5, center = true);
        
        }        

    } 
}


include <box/box_lib.scad>;

/*
translate([-8, 50, 6.5])
cube(size = [2,2,9.3], center = true/false);

translate([-5, 50, 11])
cube(size = [40,4,5], center = true/false);

translate([-35, 50, 11])
cube(size = [23,4,5], center = true/false);



translate([22.5, -54, -21])
cube(size = [17.3,4,5], center = true/false);

translate([22.5, -54, -30])
cube(size = [17.3,11,12.5], center = true/false);


            
            
            
translate([-40, -54, -30])
cube(size = [36,11,8], center = true/false); //Abstand links


translate([-22.5, -54, -30])
cube(size = [17.3,11,8], center = true/false); //Abstand unten

*/




               











