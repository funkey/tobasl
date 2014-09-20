#ifndef MULTI2CUT_INFERENCE_RECONSTRUCTOR_H__
#define MULTI2CUT_INFERENCE_RECONSTRUCTOR_H__

#include <pipeline/SimpleProcessNode.h>

#include <slices/Slices.h>
#include "Solution.h"
#include "SliceVariableMap.h"

class Reconstructor : public pipeline::SimpleProcessNode<> {

public:

	Reconstructor();

private:

	void updateOutputs();

	pipeline::Input<Slices>           _slices;
	pipeline::Input<Solution>         _solution;
	pipeline::Input<SliceVariableMap> _sliceVariableMap;

	pipeline::Output<Slices> _reconstruction;
};

#endif // MULTI2CUT_INFERENCE_RECONSTRUCTOR_H__

