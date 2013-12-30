package basic;
/*
 * Creating user defined object
 */
public class Instances2 {
	public static class Rectangle {
		private int a, b;
		public Rectangle(int a, int b) {
			this.a = a;
			this.b = b;
		}

		public int area() {
			return a*b;
		}

		public int perimeter() {
			return (a << 1) + (b << 1);
		}
	}
	// this hierarchy only make sense if the objects are inmutable
	public static class Square extends Rectangle {
		public Square(int a) {
			super(a,a);
		}
	}

	public static int test0() {
		/*
		 * Description:
		 * Primary or secondary school? oh my God, what an old guy I am :-)
		 */
		int c;
		Rectangle r = new Rectangle(12,134);
		c = r.area();

		r = new Square(15);
		return c + r.area();
	}

	public static int test1() {
		/*
		 * Description:
		 * other useless stuff
		 */

		return (new Square(13).perimeter()) * (new Rectangle(12,1).perimeter());
	}

}
