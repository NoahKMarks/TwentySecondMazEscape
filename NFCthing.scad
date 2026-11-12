$fn = 32;

color("#AA5566")

rotate(a=90, v=[0,1,0])

sphere(5);

$fn = 16;

for (i = [0:30:330]){
	bob =  str("#55",str(i%101),str(i%101));
	echo(bob);
	color(bob)

	rotate(a=i,v=[1,0,0])
	translate([0,0,5])
	cylinder(h=10, r1=1, r2 = 3);

}
