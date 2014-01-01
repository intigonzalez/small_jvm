/*
 * SimpleStack.h
 *
 *  Created on: Dec 31, 2013
 *      Author: inti
 */

#ifndef SIMPLESTACK_H_
#define SIMPLESTACK_H_

/*
 * I don't like the behavior of the STL's stack regarding top, pop.
 * I know all that speech about invariant an so on, it is nice; but it is pointless
 * when you know what you are doing and/or in classes as simple as stack.
 */

template <typename T, int N>
class SimpleStack {
private:
	T data[N];
	int idx;
public:
	SimpleStack() :idx(0) { };
	virtual ~SimpleStack() {};

	bool empty() {
		return idx == 0;
	}

	void push(T t) {
//		if (idx == N)
//			throw(std::runtime_error("The stack is full"));
		data[idx++] = t;
	}

	T& top() {
//		if (idx == 0)
//			throw(std::runtime_error("The stack is empty"));
		return data[idx-1];
	}

	T pop() {
//		if (idx == 0)
//			throw(std::runtime_error("The stack is empty"));
		return data[--idx];
	}
};

#endif /* SIMPLESTACK_H_ */
