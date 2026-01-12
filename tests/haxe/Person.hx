// haxe --run Person

class Person {
    static function main() {
        var hello = "hello";
        trace(hello);
        Sys.println(hello);
        var bob = new Bob();
        bob.hello();
    }
    public function new ():Void {
        trace("I am a person!");
    }
    function hello():Void {   
    }
}

class Bob extends Person {
    public function new ():Void {
        super();
        trace("I am Bob!");
    }
    override function hello () {
        Sys.println("Bob says hello!");
    }
}

