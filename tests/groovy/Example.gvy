/* This is a multiline comment
   which can wrap over multiple lines

*/
class Example {
    int y ;
    public int getY () {
        return(y);
    }
    public void setY (int pY) {
        y = pY;
    }
    static void incr(int a,int b=1) {
        int c=a+b;
        println(c);
    }
    static void sum(int a,int b=5) {
        int c=a+b;
        println(c);
    }
    static def DisplayName() {
        println('Hello World');
        println('New Hello!');
    }
    static void main(String[] args) {
        // Using a simple println statement to print output to the console
        def x = 5;
        DisplayName();
        sum(1,2);
        incr(6);
        Example ex = new Example();
        ex.setY(12);
        println(ex.getY());
        Student st = new Student();
        st.StudentID = 1;
        st.Marks1 = 10;
        println(st.DisplayMarks())
    }
}
interface Total {
    void DisplayTotal()
}
trait Marks implements Total {
    String DisplayMarks() {
        return("Display Marks");
    }
    void DisplayTotal() {
        println("Display Total");
    }
}

class Student implements Marks
{
    int StudentID
    int Marks1;
}
