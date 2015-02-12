#include <util/Logger.h>
#include "SlicesCollector.h"

logger::LogChannel slicescollectorlog("slicescollectorlog", "[SlicesCollector] ");

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

	// slice hash to slice id for each unique slice
	std::map<ConnectedComponentHash, unsigned int> sliceHashes;

	// slice id to slice id of already added duplicate
	std::map<unsigned int, unsigned int>           sliceCopies;

	// copy only unique slices
	foreach (boost::shared_ptr<Slices> slices, _slices) {

		foreach (boost::shared_ptr<Slice> slice, *slices) {

			ConnectedComponentHash hash = slice->getComponent()->hashValue();

			// duplicate?
			if (sliceHashes.count(hash)) {

				// remember mapping of this slice to duplicate
				sliceCopies[slice->getId()] = sliceHashes[hash];

			} else {

				// add the slice
				_allSlices->add(slice);

				// remember it via its hash
				sliceHashes[hash] = slice->getId();
			}
		}
	}

	LOG_USER(slicescollectorlog) << "removed " << sliceCopies.size() << " duplicates" << std::endl;

	// copy conflict sets
	foreach (boost::shared_ptr<ConflictSets> conflictSets, _conflictSets) {

		foreach (const ConflictSet& conflictSet, *conflictSets) {

			ConflictSet mapped;
			foreach (unsigned int sliceId, conflictSet.getSlices()) {

				// is a duplicate?
				if (sliceCopies.count(sliceId))
					mapped.addSlice(sliceCopies[sliceId]);
				else
					mapped.addSlice(sliceId);
			}

			_allConflictSets->add(mapped);
		}
	}

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
