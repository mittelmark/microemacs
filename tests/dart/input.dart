import 'dart:io';
void main () {
    // user input
    // 
    
    String hi;
    String? hi2;
    print("Enter your name:");
    // is always as nullable string
    var name = stdin.readLineSync();
    print("Hello $name!");
    print("Enter a number:");
    var number = stdin.readLineSync();
    int x = 3;
    print(x+int.parse(number ?? '0')); // as number might be null if nothing entered
    var y = "3";
    print(x+int.parse(y));
    // functions
    print('1'.toString()+'40');
}
