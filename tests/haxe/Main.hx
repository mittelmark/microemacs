import sys.io.File;
import sys.FileSystem;
import Sys;

class Main {
    static function main() {
        var args = Sys.args(); // command line arguments
        if (args.length == 0) {
            trace("Usage: myprog <filename>");
            return;
        }

        var path = args[0]; // first argument = file name

        if (FileSystem.exists(path)) {
            var content = File.getContent(path);
            trace(content);
        } else {
            trace('File not found: $path');
        }
    }
}
