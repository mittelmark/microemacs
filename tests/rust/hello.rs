use std::io;
mod test {
    pub fn run () {
        println!("Running test ...");
    }
}
// Define the Person struct with age and id fields
struct Person {
    id: u32,
    age: u8,
}
// TODO: add more methods
// Implement methods for Person
impl Person {
    // Method: eat
    fn eat(&self) {
        println!("Person with id {} is eating.", self.id);
    }
}

fn main () {
    println!("Hello World!");
    test::run();
    // Create a new Person instance
    let person = Person { id: 1, age: 30 };

    // Access properties
    println!("Person id: {}, age: {}", person.id, person.age);

    // Call the eat method
    person.eat();
    
}
