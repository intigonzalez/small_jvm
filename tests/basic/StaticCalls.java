package basic;
/*
 * Contains tests for static call manipulation
 */
public class StaticCalls {

	public static int sum(int a, int b) {
		return a + b;
	}

	public static int test0() {
		/* 
		 * Description: 
		 * Call a simple function in the same class
		 */
		int x = sum(13,14);
		int y = sum(x,123);
		int z = sum(x,y);
		int n = sum(sum(13,sum(23,23)),x+y+z);
		return n;
		
	}

	public static int factorial(int n) {
		if (n == 0)
			return 1;
		return n*factorial(n-1);
	}

	public static int test1() {
		/* 
		 * Description: 
		 * Call a recursive function
		 */
	       	return factorial(12);
	}

	public static int fib(int n) {
		// I never remember the definition
		if (n<2) return n;
		return fib(n-1) + fib(n-2);
	}

	public static int test3() {
		/* 
		 * Description:
		 * call recursive fibonnaci 
		 */
		return fib(6);
	}

	public static int otherFunction(int a, int b) {
		return factorial(b) * a;
	}

	public static int test4() {
		/* 
		 * Description:
		 * Call two functions
		 */

		int n = factorial(4);
	        int m = otherFunction(n,12);
		return m;
	}

	public static int test5() {
		/* 
		 * Description:
		 * call function in loop
i		 */
		int s = 0;
		for (int i = 0 ; i < 100000000 ; i++) {
			s = sum(s,i);
		}
		return s;
	}


	private static int bsearch(int[] a, int n, int m, int M) {
		if (m > M) return -1; // not in the array
		int mid = (m + M) >> 1;
		if (n < a[mid]) {
			return bsearch(a,n,m,mid-1);
		}
		else if (n > a[mid]) {
			return bsearch(a,n,mid+1, M);
		}
		return mid;
	}

	private static int bsearch(int[] a, int n) {
		return bsearch(a, n, 0, a.length - 1);
	}

	public static int test6() {
		/* 
		 * Description: 
		 * Call binary search, pass an array as parameter,
		 * Also, method overload 
		 */

		int[] a = new int[100000];
		for (int i = 0 ; i < a.length ; i++)
			a[i] = i;

		int idx = bsearch(a, 127);

		return idx;
	}

	private static void exchange(int[] a, int i, int j) {
		int tmp = a[i];
		a[i] = a[j];
		a[j] = tmp;
	}

	private static void quicksort(int[] numbers, int low, int high) {
		int i = low, j = high;
		int pivot = numbers[low + (high-low)/2];
		while (i <= j) {
			while (numbers[i] < pivot) {
			        i++;
			}
			while (numbers[j] > pivot) {
				j--;
			}

			if (i <= j) {
				exchange(numbers,i, j);
			        i++;
			        j--;
			}
		}
		if (low < j)
			quicksort(numbers, low, j);
		if (i < high)
			quicksort(numbers, i, high);
	}

	public static int test7() {
		/*
		 * Description:
		 * Execute the quick sort 
		 */ 
		int[] a = new int[100000];
		int seed = 13;
		for (int i = 0 ; i < a.length ; ++i) {
			a[i] = seed;
			seed = (seed + 1234511) % 11111111;
		}
		quicksort(a, 0, a.length - 1);
		int c = 0;
		for (int i = 0 ; i < a.length - 1 ; i++)
			if (a[i] <= a[i+1])
				c++;
		return c;
	}
}

