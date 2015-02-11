#include "SlicesCollector.h"

SlicesCollector::SlicesCollector() :
	_overlap(false /* don't normalize */, false /* don't align */) {

	registerInputs(_slices, "slices");
	registerInputs(_conflictSets, "conflict sets");
	registerOutput(_allSlices, "slices");
	registerOutput(_allConflictSets, "conflict sets");
}

void
SlicesCollector::updateOutputs() {

	if (!_allSlices)
		_allSlices = new Slices();
	if (!_allConflictSets)
		_allConflictSets = new ConflictSets();

	_allSlices->clear();
	_allConflictSets->clear();

	// copy slices
	foreach (boost::shared_ptr<Slices> slices, _slices)
		_allSlices->addAll(*slices);

	// copy conflict sets
	foreach (boost::shared_ptr<ConflictSets> conflictSets, _conflictSets)
		_allConflictSets->addAll(*conflictSets);

	// create new conflict sets
	for (unsigned int i = 0; i < _slices.size(); i++)
		for (unsigned int j = i + 1; j < _slices.size(); j++)
			addConflicts(*_slices[i], *_slices[j]);
}

void
SlicesCollector::addConflicts(
		const Slices& slicesA,
		const Slices& slicesB) {

	// find all overlapping slices and add pairwise conflicts
	foreach (boost::shared_ptr<Slice> sliceA, slicesA)
		foreach (boost::shared_ptr<Slice> sliceB, slicesB) {

			// overlap?
			if (!_overlap.exceeds(*sliceA, *sliceB, 0))
				continue;

			ConflictSet conflictSet;
			conflictSet.addSlice(sliceA->getId());
			conflictSet.addSlice(sliceB->getId());

			_allConflictSets->add(conflictSet);
		}
}
