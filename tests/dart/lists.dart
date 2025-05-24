void main () {
    // List
    var myList = [1,2,3];
    print(myList);
    print(myList[0]);
    // Change an item
    myList[0]=41;
    print(myList);
    // empty list;
    var el = [];
    // add single item
    el.add(31);
    print(el);
    // add multiple items
    el.addAll([1,2,3]);
    print(el);
    // insert at specific position
    el.insert(4,4);
    print(el);
    el.insertAll(5,[5,6,7]);    
    print(el);
    // mixed lists with different data types
    var ml = [1,2,3,"Hello World","Hi"];
    print(ml);
    // remove items 
    ml.remove("Hello World");
    print(ml);
    ml.removeAt(0);
    print(ml);
}
