package basic;
/*
 * Boolean conditions
 */
public class Conditions {
	public static int test0() {
		/*
		 * Description:
		 * Basic conditions
		 */ 
		int n = 2*3*5*7*11;
		int c = 0;
		if (n > 10) c++;
		if (n < 10000000) c++;
		if (n == 1234) c++;
		if (n != 1) c++;
		if (n >= 2*3*4*5*11) c++;
		if (n <= 2*3*4*5*7*11) c++;
		return c;
	}

	public static int test1() {
		/*
		 * Description:
		 * Compound conditions
		 */
		int n = 2*3*5*7*11*13;
		int c = 0;
		if (n % 2 == 0 && n % 3 == 0) c++;	// true
		if (n % 4 == 0 && n % 3 == 0) c++;	// false
		if (n % 7 == 0 || n % 3 == 14) c++;	// true
		if (n % 17 == 0 || n % 3 == 0) c++;	// true 
		if (!(n % 3 == 1)) c++;			// true
		if (!(n % 3 == 0)) c++;			// false
		if (!(n%2 == 1 || n%5==1)) c++;		// true
		return c;
	}


	private static int sum(int a, int b) {
		return a+b;
	}

	public static int test2() {
		/*
		 * Description:
		 * Include functions in condition
		 */
		if (sum(3,7) > sum(4,1) + 12)
			return 13;
		else
			return 0;
	}
}
