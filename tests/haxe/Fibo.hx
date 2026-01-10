// File: Fibo.hx

class Fibo {
    static function main() {
        var args = Sys.args();

        // Check that exactly one argument was provided
        if (args.length != 1) {
            trace("Usage: neko Main.n <non-negative integer>");
            return;
        }

        var input = args[0];
        trace(input);
        var n:Null<Int> = 0;
        // Try to parse the argument as an Int, catching errors
        try {
            n = Std.parseInt(input);
        } catch (e:Dynamic) {
            trace('Error: "$input" is not a valid integer.');
            return;
        }

        if (n == null || n < 0) {
            trace("Error: please provide a positve integer.");
            return;
        }

        trace('fib($n) = ' + fib(n));
    }

    // Simple iterative Fibonacci implementation
    static function fib(n:Int):Int {
        if (n == 0) return 0;
        if (n == 1) return 1;

        var a = 0;
        var b = 1;
        var i = 2;
        while (i <= n) {
            var tmp = a + b;
            a = b;
            b = tmp;
            i++;
        }
        return b;
    }
}
