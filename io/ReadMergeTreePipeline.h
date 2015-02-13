#ifndef MULTI2CUT_IO_READ_MERGE_TREE_PIPELINE_H__
#define MULTI2CUT_IO_READ_MERGE_TREE_PIPELINE_H__

#include "MergeTreeReader.h"
#include <inference/ProblemAssembler.h>
#include <inference/LinearSolver.h>
#include <inference/Reconstructor.h>
#include <pipeline/ProcessNode.h>
#include <loss/TopologicalLoss.h>
#include <loss/HammingLoss.h>
#include <loss/SliceDistanceLoss.h>
#include <loss/ContourDistanceLoss.h>
#include <loss/OverlapLoss.h>

/**
 * Reads a single mergetree from a given file, extracts slices and conflict 
 * sets, and -- if createBestEffort is set -- computes the best effort solution 
 * (used loss set via ProgramOption bestEffortLoss).
 */
class ReadMergeTreePipeline : public pipeline::SimpleProcessNode<> {

public:

	ReadMergeTreePipeline(
			std::string mergeTreeFile,
			bool createBestEffort);

private:

	void updateOutputs();

	// get the best-effort loss based on name
	boost::shared_ptr<pipeline::ProcessNode> getLoss(std::string name);

	pipeline::Input<Slices> _gtSlices;

	boost::shared_ptr<pipeline::ProcessNode> _bestEffortLossFunction;
	pipeline::Process<MergeTreeReader>       _mergeTreeReader;
	pipeline::Process<ProblemAssembler>      _bestEffortProblem;
	pipeline::Process<LinearSolver>          _bestEffortSolver;
	pipeline::Process<Reconstructor>         _bestEffortReconstructor;
};

#endif // MULTI2CUT_IO_READ_MERGE_TREE_PIPELINE_H__

