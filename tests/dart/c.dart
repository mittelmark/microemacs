void main () {
    Person p1 = Person("James", "Male", 41);
    Person p2 = Person("Lisa","Female",44);
    p2.showData();
    p1.showData();
}

class Person {
    String? name, sex;
    int? age;
    // constructor
    Person(String name, sex, int age) {
	this.name= name;
	this.sex = sex;
	this.age = age;
    }
    void showData() {
	print("Name: $name, Sex: $sex, Age: $age");
    }
}
