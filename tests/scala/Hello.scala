object Hello extends App {
  var x = 1;
  if (args.size == 0) {
    println("Hello Scala World!")
  } else {
    println("Hello, "+args(0)+"!")
  }
}
