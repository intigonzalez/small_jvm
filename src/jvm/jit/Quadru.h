/*
 * Quadru.h
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#ifndef QUADRU_H_
#define QUADRU_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>

namespace jit {
enum value_scope {Constant, Local, Field, Temporal, Useless, Register, Label};
enum value_type {Integer, Boolean, ObjRef,ArrRef};

struct jit_value {
	value_type type;
	value_scope scope;
	union {
		int constant; // used with constants or local variables
	} value;
	std::string toString() {
		if (scope == Constant) {
			std::ostringstream oss;
			oss << value.constant;
			return oss.str();
		}
		else if (scope == Useless)
			return "_";
		else if (scope == Temporal) {
			std::ostringstream oss;
			oss << "T" << value.constant;
			return oss.str();
		}
		else if (scope == Local) {
			std::ostringstream oss;
			oss << "L" << value.constant;
			return oss.str();
		}
		else if (scope == Label) {
			std::ostringstream oss;
			oss << "LA" << value.constant;
			return oss.str();
		}
		return "Wrong";
	}
} ;

extern jit_value  useless_value;

struct Quadr {
	unsigned char op;
	jit_value op1;
	jit_value op2;
	jit_value res;
	int label; // -1 if no label is required
	friend std::ostream& operator<< (std::ostream &out, Quadr &q);
};

struct BasicBlock {
	std::vector<Quadr> q;
};

//Define the graph using those classes
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, BasicBlock* > ControlFlowGraph;
//Some typedefs for simplicity
typedef boost::graph_traits<ControlFlowGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<ControlFlowGraph>::edge_descriptor edge_t;

struct Routine {
	unsigned countOfParameters;
	ControlFlowGraph g;
	std::vector<Quadr> q;
	int last_temp;
	std::set<int> freeTmp;

	Routine(unsigned countOfParameters);

	/**
	 * Arithmetic operations
	 */
	jit_value jit_binary_operation(unsigned char op, jit_value op1, jit_value op2);

	/**
	 * Used to emit regular quadruplos
	 */
	jit_value jit_regular_operation(unsigned char op, jit_value op1, jit_value op2, value_type result_type);
	void jit_regular_operation(unsigned char op, jit_value op1, jit_value op2, jit_value resultRef);

	/**
	 * Methods
	 */
	void jit_return_int(jit_value r);

	/**
	 * Assignments
	 */
	void jit_assign_local(jit_value local, jit_value v);


	void buildControlFlowGraph();

	/**
	 * Debug
	 */
	void print();
};

/**
 * Functions to deal with jit values
 */
jit_value jit_constant(int c);
jit_value jit_local_field(int index, value_type type);
jit_value jit_label(int pos);


}
#endif /* QUADRU_H_ */
