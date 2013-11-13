class Lista {
	class Node {
		int value = 0;
		Node next = null;
	}
	
	Node head = new Node();
	
	public void add(int n) {
		Node tmp = new Node();
		tmp.value = n;
		tmp.next = head.next;
		head.next = tmp;
	}
	
	public boolean empty() {
		return head.next == null;
	}
	
	public int removeFirst() {
		int v = head.next.value;
		head.next = head.next.next;
		return v;
	}
}