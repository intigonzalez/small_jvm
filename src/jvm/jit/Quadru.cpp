/*
 * Quadru.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#include "Quadru.h"
#include <iostream>
#include <string>

#include <boost/graph/graphviz.hpp>

using namespace std;

namespace jit {

jit_value useless_value = {
		Integer,
		Useless,
		0
};

std::ostream& operator <<(std::ostream& stream, Quadr& q) {

	if (q.label != -1)
		stream << "LA" << q.label << ":" << "\\n";
	Quadr qq = q;
	stream << '(' << q.op << "," << qq.op1.toString() << ","  << qq.op2.toString() << ","  << qq.res.toString() << ")" << "\\n" ;
	return stream;
}

/**
 * Functions to deal with jit values
 */
jit_value jit_constant(int c) {
	jit_value r = {
			Integer,
			Constant,
			0
	};
	r.value = c;
	return r;
}

jit_value jit_address(void* address) {
	jit_value r = {
				ObjRef,
				Constant,
				0
		};
	r.value = (int32_t)address;
	return r;
}

jit_value jit_null() {
	jit_value r = {
			ObjRef,
			Constant,
			0
	};
	r.value = 0;
	return r;
}

jit_value jit_local_field(int index, value_type type){
	jit_value r = {
			type,
			Local,
			0
	};
	r.value = (unsigned char)index;
	return r;
}

jit_value jit_label(int pos) {
	jit_value r = {
			Integer,
			Label,
			0
	};
	r.value = pos;
	return r;
}

Routine::Routine(unsigned countOfParameters) {
	this->countOfParameters = countOfParameters;
	last_temp = 0;
}

/**
 * Arithmetic operations
 */
jit_value Routine::jit_binary_operation(OP_QUAD op, jit_value op1, jit_value op2) {
	Quadr r;
	if (op1.meta.type == op2.meta.type) {
		r.op1 = op1;
		r.op2 = op2;
		r.op = op;
		r.res.meta.type = op1.meta.type;
		r.res.meta.scope = Temporal;
		r.label = -1;
		r.res.value = getTempId();
		q.push_back(r);
		if (op1.meta.scope == Temporal)
			freeTmp.insert(op1.value);
		if (op2.meta.scope == Temporal)
			freeTmp.insert(op2.value);
		return r.res;
	}
	else throw new std::exception();
}

/**
 * Used to emit regular quadruplos
 */
jit_value Routine::jit_regular_operation(OP_QUAD op, jit_value op1, jit_value op2, value_type result_type) {
	Quadr r;
	r.op1 = op1;
	r.op2 = op2;
	r.op = op;
	r.res.meta.type = result_type;
	r.res.meta.scope = Temporal;
	r.label = -1;
	r.res.value = getTempId();
	q.push_back(r);
	if (op1.meta.scope == Temporal)
		freeTmp.insert(op1.value);
	if (op2.meta.scope == Temporal)
		freeTmp.insert(op2.value);
	return r.res;
}

/**
 * Used to emit regular quadruplos
 */
void Routine::jit_regular_operation(OP_QUAD op, jit_value op1, jit_value op2, jit_value resultRef) {
	Quadr r;
	r.op1 = op1;
	r.op2 = op2;
	r.op = op;
	r.res = resultRef;
	r.label = -1;
	q.push_back(r);
	if (op1.meta.scope == Temporal)
		freeTmp.insert(op1.value);
	if (op2.meta.scope == Temporal)
		freeTmp.insert(op2.value);
}

/**
 * Methods
 */
void Routine::jit_return_int(jit_value r) {
	Quadr result = {
		OP_RETURN,
		r,
		useless_value,
		useless_value,
		-1
	};
	q.push_back(result);
}

void Routine::jit_return_void(void) {
	Quadr result = {
		OP_RETURN,
		useless_value,
		useless_value,
		useless_value,
		-1
	};
	q.push_back(result);
}

/**
 * assignaments
 */
void Routine::jit_assign_local(jit_value local,jit_value v) {
	Quadr result = {
		ASSIGN,
		v,
		useless_value,
		local,
		-1
	};
	if (v.meta.scope == Temporal)
		freeTmp.insert(v.value);
	q.push_back(result);
}

void Routine::buildControlFlowGraph() {
	map<int , vertex_t> map;
	vertex_t lastV ;
	bool nextIsNewBlock = true;
	for (vector<Quadr>::iterator it = q.begin(), itEnd = q.end() ; it != itEnd ; ++it ) {
		vertex_t vertex;
		if ((*it).label != -1) {
			// any label is the start of a basic block
			std::map<int , vertex_t>::iterator ir = map.find((*it).label);
			if (ir == map.end()) {
				vertex = boost::add_vertex(g);
				g[vertex] = new BasicBlock();
				map.insert( pair<int, vertex_t>((*it).label, vertex));
			}
			else vertex = (*ir).second;

			if ((*it).label != 0) {
				 // the connection only exist if the previous instruction was not a goto
				BasicBlock* block = g[lastV];
				if (block->q[block->q.size() - 1].op != 1)
					boost::add_edge(lastV, vertex, g);
			}
			lastV = vertex;
		} else if (nextIsNewBlock) {
			vertex = boost::add_vertex(g);
			g[vertex] = new BasicBlock();
			if (it != q.begin())
				boost::add_edge(lastV, vertex, g);
			lastV = vertex;
		}
		nextIsNewBlock = false;
		if ((*it).res.meta.scope == Label) {
			// after any jmp (conditional or not) there is a new basic block
			// create/access basic block for jump target
			std::map<int , vertex_t>::iterator ir = map.find((*it).res.value);
			if (ir == map.end()) {
				vertex = boost::add_vertex(g);
				g[vertex] = new BasicBlock();
				map.insert( pair<int, vertex_t>((*it).res.value, vertex));
			}
			else vertex = (*ir).second;

			boost::add_edge(lastV, vertex, g);
			// if not a GOTO then mark the next instruction as BasicBlock start
			nextIsNewBlock = (*it).op != 1;
		}
		BasicBlock* block = g[lastV];
		block->q.push_back(*it);
	}
}

/**
 * Utils
 */
int Routine::getTempId() {
	if (freeTmp.empty())
		return last_temp++;
	else {
		std::set<int>::iterator it = freeTmp.begin();
		int id = *it;
		freeTmp.erase(id);
		return id;
	}
}

/**
 * Debug
 */
class label_writer {
  public:
    label_writer(ControlFlowGraph& g) : graph(g) {}
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& v) const {
    	BasicBlock* bb = graph[v];
    	out << "[label=\"" ;
    	out << "Block " << v << "\\n";
    	for (unsigned i = 0 ; i < bb->q.size() ; ++i) {
    		out << bb->q[i] << "\\n";
    	}
    	out << "\"]";
    }
  private:
    ControlFlowGraph& graph;
  };

label_writer
make_label_writer(ControlFlowGraph& g) {
	return label_writer(g);
}

void Routine::print() {
	ofstream file("in.txt");
	boost::write_graphviz(file, g, make_label_writer(g));
	file.close();
	std::cout <<  "dsffsdf " <<  sizeof(Quadr) << " " << sizeof(DataQuad) << " " <<  sizeof(jit_value) << std::endl;
	for (unsigned i = 0 ; i < q.size() ; i++) {
		Quadr tmp = q[i];
		if (tmp.label != -1)
			std::cout << "LA" << tmp.label << ":" << endl;
		std::cout << '(' << tmp.op << "," << tmp.op1.toString() << ","  << tmp.op2.toString() << ","  << tmp.res.toString() << ")" << endl ;
	}
}

}

