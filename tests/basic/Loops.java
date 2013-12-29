package basic;
/*
 * This class contains tests for simple java loops
 */
public class Loops {

	public static int test0() {
		int c = 0;
		for (int i = 0 ; i <= 100 ; i++)
			c+=i;
		return c;
	}

	public static int test1() {
		int c = 0;
		int i = 0;
		while (i<=100) {
			c+=i++;
		}
		return c;
	}

	public static int test2() {
		// sum of the first 100 prime number
		int c = 0;
		int sum = 0;
		int current = 2;
		while (c <= 100) {
			int tmp = 1;
			for (int i = 2 ; i < current ; ++i) {
				if (current % i == 0)
					tmp = 0;
			}
			if (tmp == 1) {
				// is prime
				sum += current;
				c++;
			}
			current++;
		}
		return sum;
	}

	public static int test3() {
		// calculating 3^13, clever algorithm
		// 	 | a^(b/2)*a^(b/2) 	if b is even
		// a^b = | a + a^(b/2)*a^(b/2)  if b is odd
		//	 | 1 			if b = 0
		//
		// or in iterative way
		// since we can represent b as binary
		// a^b = a^(2^n) * a^(2^m) * ... * a^(2^p)
		// where n > m > ... > p and they are index on b's binary representation such that b-{n|m|...|p}=1  
		// 13^7 = 13^(2^2)*13^(2^1)*13^(2^0)
		int a = 3, b = 13;
		
		int p = 1;
		int v = a;
		while (b > 0) {
			if (b % 2 == 1)
				p *= v;
			b = b / 2; // a better option is >> but it is not yet implemented
			v=v*v;
		}
		return p;
	}

	public static int test4() {
		// calculating 3^13, naive algorithm
		int a = 3, b = 13;

		int p = 1;
		for (int i = 0 ; i < b ; i++)
			p *= a;
		return p;
	}
	
	public static int test5() {
		/* Description
		 * Nested loops 
		 */ 

		int c = 0;
		for (int i = 0 ; i < 100; ++i) {
			for (int j = i ; j < 2*i ; j++) {
				for (int k = 10 ; k < 1000 ; k++)
					c ++;
			}
		}

		// now fibo(c)
		int f0 = 0;
		int f1 = 1;
		int index = 2;
		while (index <= c) {
			int tmp = f0 + f1;
			f0 = f1;
			f1 = tmp;
			index++;
		}
		return f1;
	}
}
