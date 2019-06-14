use GD;

$wi=1440;
$he=2560;


$image=GD::Image->new($wi,$he,1);
$tmp=GD::Image->new($wi,$he,1);

           # allocate some colors
           $white = $image->colorAllocate(255,255,255);
           $black = $image->colorAllocate(0,0,0);


$image->filledRectangle(0,0,$wi,$he,$black);
$image->rectangle(0,0,$wi-1,$he-1,$white);

$pos=0;
$size=1;
while($pos<$he){
$image->filledRectangle(0,$pos,$wi,$pos+$size-1,$white);

$fontsize=$size;
if($fontsize<10){$fontsize=10;}

@bounds=$tmp->stringFT(0,"/usr/share/wine/fonts/tahoma.ttf",$fontsize,0,0,0,$size);
$offset=-$size+$bounds[1]-2;
print "$size= $bounds[1]\n";
for($w=-10;$w<=10;$w++){
for($q=-10;$q<=10;$q++){
$image->stringFT($white,"/usr/share/wine/fonts/tahoma.ttf",$fontsize,0,$q,$pos+$w-$offset,$size);
}
}
push(@txt,[$fontsize,$pos-$offset,$size]);

$size++;

$interval=$size;
if($interval<10){
$interval=10;
}
if($interval>20){
$interval=20;
}
$pos+=$size+$interval;

}

foreach(@txt){
$image->stringFT($black,"/usr/share/wine/fonts/tahoma.ttf",$_->[0],0,0,$_->[1],$_->[2]);
}





open(dd,">stripe.png");
print dd $image->png;
