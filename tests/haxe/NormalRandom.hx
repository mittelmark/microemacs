// https://www.perplexity.ai/search/how-can-i-create-random-data-w-_10jU2EBT3CGK_wtX.kI3w
// https://www.perplexity.ai/search/how-can-i-create-random-data-w-_10jU2EBT3CGK_wtX.kI3w
class NormalRandom {
    static public function normal(mean:Float = 0, stdDev:Float = 1):Float {
        // Generate two uniform random numbers
        var u1:Float = Math.random();
        var u2:Float = Math.random();
        
        // Box-Muller transform to standard normal (mean 0, std dev 1)
        var r:Float = Math.sqrt(-2 * Math.log(u1));
        var theta:Float = 2 * Math.PI * u2;
        var z:Float = r * Math.cos(theta);  // or sine for the paired value
        
        // Scale and shift: N(mean, stdDev)
        return mean + stdDev * z;
    }
    
    // Generate array of N samples
    static public function generate(n:Int, mean:Float = 0, stdDev:Float = 1):Array<Float> {
        var samples:Array<Float> = [];
        for (i in 0...n) {
            samples.push(normal(mean, stdDev));
        }
        return samples;
    }
}
