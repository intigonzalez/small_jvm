/*
 * graph.h
 *
 *  Created on: Jan 7, 2014
 *      Author: inti
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <vector>
#include <string>
#include <ostream>

template <typename I>
class Edge {
public:
	int trg;
	I info;
	Edge(int t, I i): trg(t), info(i) {}

};

template <typename N, typename I>
class graph {
public:

	typedef typename std::vector<N>::iterator iteratorNodes;
	typedef typename std::vector<Edge<I>>::iterator iteratorEdges;
	typedef Edge<I> edgeType;


	std::vector< std::vector<Edge<I>> > edges;
	std::vector<N> nodes;

	void addVertex(N v) {
		nodes.push_back(v);
		std::vector<Edge<I>> edge;
		edges.push_back(edge);
	}

	void printGraphViz(std::ostream& output, std::string& graph_name) {
		output << "digraph " << graph_name << " {\n";
		for (size_t i = 0 ; i < nodes.size() ; i++)
			output << "\t" << "v" << i << " [labeljust=l,label=\"" << nodes[i]->getVizLabel() <<"\",shape=box];\n";
		for (size_t i = 0 ; i < nodes.size() ; i++)
			for (size_t j = 0 ; j < edges[i].size() ; ++j) {
				output << "\t" << "v" << i << "-> v" << edges[i][j].trg << ";\n";
			}
		output << "}\n";
	}

	iteratorNodes begin() {
		return nodes.begin();
	}

	iteratorNodes end() {
		return nodes.end();
	}

	iteratorEdges outputs_begin(int v) {
		return edges[v].begin();
	}

	iteratorEdges outputs_end(int v) {
		return edges[v].end();
	}

};

#endif /* GRAPH_H_ */
