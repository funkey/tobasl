#ifndef MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__
#define MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include <slices/ConflictSets.h>
#include <features/Features.h>
#include <loss/LossFunction.h>

class LearningProblemWriter : public pipeline::SimpleProcessNode<> {

public:

	/**
	 * Create a new LearningProblemWriter that writes the learning problem 
	 * description in the given directory.
	 */
	LearningProblemWriter(std::string directory = "./");

	void write();

private:

	void updateOutputs() {}

	void getSliceCosts();

	pipeline::Input<Slices>       _slices;
	pipeline::Input<ConflictSets> _conflictSets;
	pipeline::Input<Features>     _features;
	pipeline::Input<Slices>       _bestEffort;
	pipeline::Input<LossFunction> _lossFunction;

	std::string _directory;
};

#endif // MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

