public class Dos {
	
	static class Cuadrado {
		int a;
		Cuadrado(int n) {
			a = n;	
			//color++;
		}

		int area() {
			return a*a; 		
		}
	}

	public static int testingNew() {
		Cuadrado c = new Cuadrado(12);
		return c.area();
	}
}
