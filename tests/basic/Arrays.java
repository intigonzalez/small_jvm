package basic;
/*
 * Contains tests for raw array manipulation
 */
public class Arrays {
	public static int test0() {
		/* 
		 * Description: 
		 * Create array, assign value to elements, read elements and access length
		 * */ 
		int[] a = new int[3000];
		for (int i = 0 ; i < a.length ; i++)
			a[i] = i;

		int sum = 0;
		for (int i = a.length-1; i >= 0 ; --i) 
			sum += a[i];
		return sum;
	}

	public static int test1() {
		/* 
		 * Description: 
		 * Sort array with direct insertion
		 */
	       	int[] a = new int[100];
		for (int i = a.length - 1 ; i >= 0 ; i--) 
			a[i] = i;

		// sort
		for (int i = 0 ; i < a.length ; ++i) {
			int min = a[i];
			int imin = i;
			for (int j = i+1 ; j < a.length ; ++j) {
				if (a[j] < min) {
					imin = j;
					min = a[j];
				}
			}
			a[imin] = a[i];
			a[i] = min;
		}
		int sorted = 0;
		for (int i = 0 ; i < a.length - 1 ; i++) {
			if (a[i] <= a[i+1] )
				sorted++;
		}
		if (sorted == a.length - 1)
			return 115;
		else
			return 0;
	}

	public static int test2() {
		/* 
		 * Description:
		 * La criba the 
		 */
		int[] primes = new int[10000];
		int c = 0;
		int[] numbers = new int[100000];
		for (int p = 2 ; p < numbers.length ; ++p) {
			if (numbers[p] != 1) {
				// p is prime
				primes[c++] = p;
				for (int j = p*2 ; j < numbers.length ; j+=p) // in fact j can starts with p*p but ... arithmetic overflow
					numbers[j] = 1;
			}
		}
		int mul = 1;
		for (int i = 0 ; i < c ; i++)
			mul *= primes[i];
		return mul;
	}

	public static int test3() {
		/* 
		 * Description:
		 * Playing with references
		 */
	       	int[] a = new int[123];
		for (int i = 0 ; i < a.length ; i++) 
			a[i] = 2*i;
		int[] b = a;
		int[] c = new int[a.length * 3];
		for (int i = 0 ; i < b.length ; i++) {
			c[i+10] = b[i] + a[i];
		}
		int sum = 0;
		for (int i = 0 ; i < c.length ; ++i)
			sum -= c[i];	
		return -sum;
	}

	public static int test4() {
		/* 
		 * Description:
		 * Playing with references
i		 */
	       	int[] a = new int[123];
		for (int i = 0 ; i < a.length ; i++) 
			a[i] = 2*i;
		int[] b = a;
		int[] c = new int[a.length * 3];
		for (int i = 0 ; i < b.length ; i++) {
			c[i+10] = b[i] + a[i];
			a[i] = b[i] = 14;
		}
		int sum = 0;
		for (int i = 0 ; i < c.length ; ++i)
			sum -= c[i];	
		return -sum;
	}


	public static int test5() {
		/* 
		 * Description: 
		 * Calculate Triangle of Pascal and then Comb(12,5)
		 */ 
		int[][] a = new int[12][12];
		for (int i = 0 ; i < a.length ; i++)
			a[i][0] = a[i][i] = 1;

		return 0;
	}

	public static int test6() {
		/* 
		 * Description: 
		 * Create char array and manipulate it
		 */ 
		char[] a = new char[123456];
		char c = 'a';
		for (int i = 0; i < a.length; ++i) {
			a[i] = c;
			c ++;
			if (c > 'z')
				c = 'a';
		}
		int count = 0; // numer of a
		for (int i = 0; i < a.length ; i++) {
			if (a[i] == 'a')
				count++;
		}
		return count;
	}

	public static int test7() {
		/* 
		 * Description: 
		 * Array of booleans
		 */
		boolean[] b = new boolean[4500];
		for (int i = 0; i < b.length ; i++) {
			b[i] = i % 13 == 5;
		}
		int count = 0;
		for (int i = 0; i < b.length; i++)
			count += (b[i]?1:0);
		return count;
	}

	public static int test8() {
		/* 
		 * Description 
		 * Array of bytes
		 */
		byte[] b = new byte[5];
		for (int i = 0; i < b.length ; i++)
			b[i] = (byte)(i % 125);
		int m = 1;
		for (int i = 0 ; i < b.length; i++)
			m *= b[i];
		return m;
	}
}

