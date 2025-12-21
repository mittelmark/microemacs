object Hello extends App {
  var x = 1;
  def test (): Unit = {
    println("Hello some test function!")
  }

  if (args.size == 0) {
    println("Hello Scala World!")
  } else {
    println("Hello, "+args(0)+"!")
  }
  test()
}
