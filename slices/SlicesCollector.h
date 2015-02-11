#ifndef MULTI2CUT_SLICES_SLICES_COLLECTOR_H__
#define MULTI2CUT_SLICES_SLICES_COLLECTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include <features/Overlap.h>
#include "Slices.h"
#include "ConflictSets.h"

/**
 * Collect slices from multiple Slices into a single Slices. A ConflictSet is 
 * created for each pair of overlapping slices.
 */
class SlicesCollector : public pipeline::SimpleProcessNode<> {

public:

	SlicesCollector();

private:

	void updateOutputs();

	void addConflicts(
			const Slices& slicesA,
			const Slices& slicesB);

	pipeline::Inputs<Slices>       _slices;
	pipeline::Inputs<ConflictSets> _conflictSets;
	pipeline::Output<Slices>       _allSlices;
	pipeline::Output<ConflictSets> _allConflictSets;

	Overlap _overlap;
};

#endif // MULTI2CUT_SLICES_SLICES_COLLECTOR_H__

