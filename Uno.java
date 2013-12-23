public class Uno {
	static class Cuadrado {
		static int color = 3;
		static  {
			color = 7;
		}
		
		int a;
		Cuadrado(int n) {
			a = n;	
			color++;
		}
		int area() {
			return a*a; 		
		}
	}

	public static void main(String[] a) {
		/*if (a instanceof String[])
			System.out.println("dfafd");
		int s = 0;
		int n = 100;
		for (int i = 0 ; i < n ; i++)
			s += i;
		int s2 = (n*suma(n,1))/2;
		*/
		/*for (int i = 0 ; i < 2000000 ; i++) {
			int result = verySimple(123);
			System.out.println("Result : " + result);
		}
		*/
		int[]aa = new int[100];
		aa[1] = 1;
		for (int i = 0 ; i < 2 ; i++) {
 			int r = accessingArray(1, aa);
			r = accessingArray(1, aa);
			r = accessingArray(1, aa);
		}
		//System.out.println(r);
	}

	public static int algo() {
		char[] ac2 = {'a'};
		Object o = ac2;
		//String s = new String(ac2);
		char[] ac = new char[10];
		ac[0] = 'a';
		ac[1] = ac[0];
		
		int [] arr = new int[100];
		for (int i = 0 ; i < arr.length ; i++)
			arr[i] = i *3;
		Lista l = new Lista();
		for (int i = 0 ; i < 100 ; i++)
			l.add(i);
		Cuadrado a = null;
		int n = 0;
		for (int i = 0 ; n < 100 ; ) {
			a = new Cuadrado(i);
			i++;			
			if (i == 5*1024) {
				i = 0;
				n++;
			}
			
		}
		
		int suma = 0;
		//while (!l.empty()) suma+=l.removeFirst(); 
		//return suma + Cuadrado.color;
		for (int i = 0 ; i < arr.length ; i++)
			suma += arr[i];
		
		return suma;
	}
	
	public static int cojone() {
		//System.out.println(12);
		int [] arr = new int[100];
		for (int i = 0 ; i < arr.length ; i++)
			arr[i] = i *3;
		int suma = 0;
		//while (!l.empty()) suma+=l.removeFirst(); 
		//return suma + Cuadrado.color;
		for (int i = 0 ; i < arr.length ; i++)
			suma += arr[i];
		int n = 0;
		for (int i = 0 ; n < 1000 ; ) {
			i++;			
			if (i == 5*10240) {
				i = 0;
				n++;
			}
			
		}
		return suma(3,suma);
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
	
	public static int accessingArray(int i, int[] a) {
		a[i] = a.length;
		for (int j = 0 ; j < 400000 ; j++) {
		 a[i] += j;
		 int x = 3;
		 //while (x < j) x++;
		 //if (x > 10)
		 //	a[i] = 4;
		 	
		 	int fib0 = 0;
		 	int fib1 = 1;
		 	// calculate fib10
		 	//int index = 10000;
		 	//int actual = 2;
		 	//while (actual <= index) {
		 	//	int tmp = fib0 + fib1;
		 	//	fib0 = fib1;
		 	//	fib1 = tmp;
		 	//	actual++;
		 	//}
		 	// in fib1 we have the desired value
		}
		return a[0] + a[1] + a[i];
	}

	public static int factorial(int a) {
		if (a == 0) return 1;
		return a * factorial(a - 1);	
	}
}
