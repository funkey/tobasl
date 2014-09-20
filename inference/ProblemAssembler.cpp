#include "ProblemAssembler.h"

ProblemAssembler::ProblemAssembler() {

	registerInput(_slices, "slices");
	registerInput(_conflictSets, "conflict sets");
	registerInput(_sliceCosts, "slice costs");

	registerOutput(_objective, "objective");
	registerOutput(_linearConstraints, "linear constraints");
	registerOutput(_sliceVariableMap, "slice variable map");
}

void
ProblemAssembler::updateOutputs() {

	_sliceVariableMap  = new SliceVariableMap();
	_objective         = new LinearObjective(_slices->size());
	_linearConstraints = new LinearConstraints();

	// create the objective and remember the variable mapping

	unsigned int nextVarNum = 0;
	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		unsigned int varNum = nextVarNum;
		nextVarNum++;

		_sliceVariableMap->associate(slice->getId(), varNum);

		_objective->setCoefficient(varNum, _sliceCosts->getCosts(slice->getId()));
	}

	// create one linear constraint per conflict set

	foreach (const ConflictSet& conflictSet, *_conflictSets) {

		LinearConstraint constraint;

		foreach (unsigned int sliceId, conflictSet.getSlices()) {

			unsigned int varNum = _sliceVariableMap->getVariableNum(sliceId);

			constraint.setCoefficient(varNum, 1.0);
		}

		constraint.setRelation(LessEqual);
		constraint.setValue(1.0);

		_linearConstraints->add(constraint);
	}
}

