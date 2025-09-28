// Hello World Program
// compile: kotlinc hello.kt
// run: kotlin HelloKt

// void function
fun hello(h: String) {
    println("Hello $h !")
}
// normal function

fun add( x: Int, y: Int): Int {
    return(x+y)
}
// one-line function
fun add2( x: Int,  y: Int) = x + y

// Definition of an example class
class ExampleClass(
    x: Float,
    var text: String
) {
    // Attribute werden wie eine Variable in der Klasse definiert
    val zahl = x.toInt()

    // init-Block
    init {
        println("Class was initialised with Float $x")
    }

    // Interne Methode
    fun textReverse() {
        text = text.reversed()
    }

    // Method with return
    fun firstXletters(): String {
        return(text.slice(0..zahl))
    }
}

fun main(args: Array<String>) {
    println("Hello, World!")
    println(add(3, 5))
    hello("World")
    val obj = ExampleClass(4f, "Hallo Wiki")
    println("Number is ${obj.zahl}, Text is ${obj.text}")
    obj.textReverse()
    println("Reversed text: ")
    println(obj.text)
    println("First text letters: ")
    println(obj.firstXletters())
}
