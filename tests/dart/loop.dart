void main () {
    var num = 5;
    // for loop
    for (var i = num; i > 0; i--) {
	print(i);
    }
    // for in loop
    var names = ["John", "Steve", "Jimmy"];
    for (var n in names) {
	print(n);
    }
    // while loop
    while (num >= 1) {
	print(num);
	num--;
    }
}
