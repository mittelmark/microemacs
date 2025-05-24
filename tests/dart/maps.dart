void main () {
    // maps as key value pairs
    var  toppings = {"John": "Pepperoni",
	"Marry": "Cheese"};
    print(toppings);
    print(toppings["John"]);
    // keys
    print(toppings.keys);
    // values
    print(toppings.values);    
    // length
    print(toppings.length);
    // adding items
    toppings["Tim"]="Sausage";
    print(toppings);
    toppings.addAll({"Tina": "Pizza", "Steve": "Emtpyness"});
    print(toppings);
    // removing items
    toppings.remove("Steve");
    print(toppings);
    // clear every thing
    toppings.clear();
    print(toppings.length);
    
}
