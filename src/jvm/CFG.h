/*
 * CFG.h
 *
 *  Created on: Jan 7, 2014
 *      Author: inti
 */

#ifndef CFG_H_
#define CFG_H_

#include "../utilities/graph.h"
#include "jit/Quadru.h"


namespace jvm {

enum cfg_transition_type {
	jmp_transition, code_order_transition
};

class CFG: public graph<jit::BasicBlock*, cfg_transition_type> {
public:
	CFG();
	virtual ~CFG();

	int getBBWithLabel(int l) {
		for (size_t i = 0 ; i < nodes.size(); ++i)
			if (nodes[i]->label == l)
				return i;
		return -1;
	}
};

} /* namespace jvm */

#endif /* CFG_H_ */
