#ifndef MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__
#define MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include <slices/ConflictSets.h>
#include <features/Features.h>

class LearningProblemWriter : public pipeline::SimpleProcessNode<> {

public:

	LearningProblemWriter();

	void write(const std::string& filename);

private:

	void updateOutputs() {}

	pipeline::Input<Slices>       _slices;
	pipeline::Input<ConflictSets> _conflictSets;
	pipeline::Input<Features>     _features;
};

#endif // MULTI2CUT_IO_LEARNING_PROBLEM_WRITER_H__

