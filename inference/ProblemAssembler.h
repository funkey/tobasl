#ifndef MULTI2CUT_PROBLEM_ASSEMBLER_H__
#define MULTI2CUT_PROBLEM_ASSEMBLER_H__

#include <pipeline/SimpleProcessNode.h>

#include <slices/Slices.h>
#include <slices/ConflictSets.h>

#include "LinearObjective.h"
#include "LinearConstraints.h"
#include "SliceCosts.h"
#include "SliceVariableMap.h"

class ProblemAssembler : public pipeline::SimpleProcessNode<> {

public:

	ProblemAssembler();

private:

	void updateOutputs();

	pipeline::Input<Slices>       _slices;
	pipeline::Input<ConflictSets> _conflictSets;
	pipeline::Input<SliceCosts>   _sliceCosts;

	pipeline::Output<LinearObjective>   _objective;
	pipeline::Output<LinearConstraints> _linearConstraints;
	pipeline::Output<SliceVariableMap>  _sliceVariableMap;
};

#endif // MULTI2CUT_PROBLEM_ASSEMBLER_H__

