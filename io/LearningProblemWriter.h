#ifndef MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__
#define MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/SlicesTree.h>
#include <slices/ConflictSets.h>
#include <features/Features.h>

class LearningProblemWriter : public pipeline::SimpleProcessNode<> {

public:

	LearningProblemWriter();

	void write();

private:

	void updateOutputs() {}

	std::map<unsigned int, double> getSliceCosts();

	unsigned int traverse(boost::shared_ptr<SlicesTree::Node> node, std::map<unsigned int, double>& costs, bool aboveBestEffort);

	pipeline::Input<SlicesTree>   _slices;
	pipeline::Input<ConflictSets> _conflictSets;
	pipeline::Input<Features>     _features;
	pipeline::Input<Slices>       _bestEffort;
};

#endif // MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

