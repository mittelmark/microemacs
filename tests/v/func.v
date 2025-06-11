// -!- v-lang -!-

// class like struct
struct User {
    name  string
}
// static method - constructor
fn User.new(name string) User {
    return User{ name: name}
}

// object method
fn (u User) sing() string {
    return "La, la, la ..."
}
pub struct Person {
    
}

pub fn add (x int, y int) int {
    return(x+y)
}

fn main () {
    println("Hello!")
    println(add(3,2))
    user := User.new("Mike")
    println(user.name)
    println(user.sing())

}

