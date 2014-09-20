#include "Reconstructor.h"

Reconstructor::Reconstructor() {

	registerInput(_slices, "slices");
	registerInput(_solution, "solution");
	registerInput(_sliceVariableMap, "slice variable map");

	registerOutput(_reconstruction, "reconstruction");
}

void
Reconstructor::updateOutputs() {

	_reconstruction = new Slices();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		unsigned int variableNum = _sliceVariableMap->getVariableNum(slice->getId());

		if ((*_solution)[variableNum] == 1)
			_reconstruction->add(slice);
	}
}
