/* This is some comment */
class Base {
  greet(): void {
    console.log("Hello, world!");
  }
}
 
class Derived extends Base {
  greet(name?: string): void {
    if (name === undefined) {
      super.greet();
    } else {
      console.log(`Hello, ${name.toUpperCase()}`);
    }
  }
}

enum CardinalDirections {
  North,
  East,
  South,
  West
}
const car = {
  type: "Toyota",
};
function greet(name: string): string {
  return `Hello, ${name}!`;
}

type Animal = { name: string };
type Bear = Animal & { honey: boolean };
const bear: Bear = { name: "Winnie", honey: true };

type CarYear = number
type CarType = string
type CarModel = string
type Car = {
  year: CarYear,
  type: CarType,
  model: CarModel
}
let isActive: boolean = true;
let hasPermission = false; // TypeScript infers 'boolean' type 
const message: string = greet("World");
console.log(message);
const d = new Derived();
d.greet();
d.greet("reader");
console.log(greet("Function"));

interface Rectangle {
  height: number,
  width: number
}

const rectangle: Rectangle = {
  height: 20,
  width: 10
};

interface ColoredRectangle extends Rectangle {
  color: string
}

const coloredRectangle: ColoredRectangle = {
  height: 20,
  width: 10,
  color: "red"
};

class Person {
  private name: string;

  public constructor(name: string) {
    this.name = name;
  }

  public getName(): string {
    return this.name;
  }
}

const person = new Person("Jane");
console.log(person.getName()); // person.name isn't accessible from outside the class since it's private

