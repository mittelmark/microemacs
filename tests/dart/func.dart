int add (x,y) {
    return x+y;
}
void main () {
    String myFunc (name,{String name1="Friends", String name2="Hi"}) {
	return "Hello Function! $name2 $name and $name1!";
	
    }
    print(myFunc("John"));
    print(myFunc("John",name1:"Enemies",name2:"Welcome"));
    print(add(1,2));
}
