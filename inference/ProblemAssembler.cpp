#include "ProblemAssembler.h"

ProblemAssembler::ProblemAssembler() {

	registerInput(_slices, "slices");
	registerInput(_conflictSets, "conflict sets");

	// one of them has to be set
	registerInput(_sliceCosts, "slice costs", pipeline::Optional);
	registerInput(_sliceLoss, "slice loss", pipeline::Optional);

	registerOutput(_objective, "objective");
	registerOutput(_linearConstraints, "linear constraints");
	registerOutput(_sliceVariableMap, "slice variable map");
}

void
ProblemAssembler::updateOutputs() {

	if (!_sliceCosts.isSet() && !_sliceLoss.isSet())
		UTIL_THROW_EXCEPTION(
				UsageError,
				"at least one of slice costs or slice loss has to be set");

	_sliceVariableMap  = new SliceVariableMap();
	_objective         = new LinearObjective(_slices->size());
	_linearConstraints = new LinearConstraints();

	// create the objective and remember the variable mapping

	unsigned int nextVarNum = 0;
	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		unsigned int varNum = nextVarNum;
		nextVarNum++;

		_sliceVariableMap->associate(slice->getId(), varNum);

		if (_sliceCosts.isSet())
			_objective->setCoefficient(varNum, _sliceCosts->getCosts(slice->getId()));
		else
			_objective->setCoefficient(varNum, (*_sliceLoss)[slice->getId()]);
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

