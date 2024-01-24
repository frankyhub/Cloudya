//Platinen Pin 

$fn=80;
Rgr = 3.5;  //Radius PIN 
R1kl = 1.7; //Radius 1 Stift
R2kl = 1.5; //Radius 2 Stift
Sh = 5.8;     //Sockelhöhe

color("WhiteSmoke")
cylinder(h = Sh, r1=Rgr , r2=Rgr , center = true); //Sockel
translate(v = [0, 0, Sh/2])
color("WhiteSmoke")
cylinder(h = 3, r1 = R1kl, r2 = R2kl, center = true); //Pin

