public class Uno {
	
	
	public static int tonto() {
		return 13;
	}
	
	public static int tonto1() {
		int x = 22;
		int t = x * x;
		int b = suma(2, 5);
		return 13 + t + x * 2 + b;
	}
	
	public static int suma(int a, int b) {
		return a + b;	
	}
	
	public static int verySimple() {
		int x = 123;
		x = 45 + x; 
		int y = 13;
		int n = 56;
		
		return (3 + 4 + x + 2*n + y*x - 1) / 7;
		//return x / 5;
	}
	
	public static int verySimple(int number) {
		int x = number;
		x = 45 + x; 
		int y = 13;
		int n = 56;
		n = n * 2;
		return (3 + 4 + x + 2*n + y*x - 1) / 7;
		//return x / 5;
	}
	
}
