#include "ReadMergeTreePipeline.h"
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>

logger::LogChannel readmergetreepipelinelog("readmergetreepipelinelog", "[ReadMergeTreePipeline] ");

util::ProgramOption optionBestEffortLoss(
		util::_long_name        = "bestEffortLoss",
		util::_description_text = "The candidate loss function to use to find the best-effort for learning. Valid values are: "
		                          "'contourdistance', 'overlap' (default), and 'slicedistance'.",
		util::_default_value    = "overlap");

ReadMergeTreePipeline::ReadMergeTreePipeline(
		std::string mergeTreeFile,
		bool createBestEffort) :
	_mergeTreeReader(mergeTreeFile) {

	// setup internal pipeline
	if (createBestEffort) {

		LOG_USER(readmergetreepipelinelog) << "creating best-effort loss..." << std::endl;
		_bestEffortLossFunction = getLoss(optionBestEffortLoss);

		_bestEffortProblem->setInput("slices", _mergeTreeReader->getOutput("slices"));
		_bestEffortProblem->setInput("conflict sets", _mergeTreeReader->getOutput("conflict sets"));
		_bestEffortProblem->setInput("slice loss", _bestEffortLossFunction->getOutput());

		pipeline::Value<LinearSolverParameters> linearSolverParameters;
		linearSolverParameters->setVariableType(Binary);
		_bestEffortSolver->setInput("objective", _bestEffortProblem->getOutput("objective"));
		_bestEffortSolver->setInput("linear constraints", _bestEffortProblem->getOutput("linear constraints"));
		_bestEffortSolver->setInput("parameters", linearSolverParameters);

		_bestEffortReconstructor->setInput("slices", _mergeTreeReader->getOutput());
		_bestEffortReconstructor->setInput("slice variable map", _bestEffortProblem->getOutput("slice variable map"));
		_bestEffortReconstructor->setInput("solution", _bestEffortSolver->getOutput("solution"));
	}

	// inputs
	if (createBestEffort)
		registerInput(_bestEffortLossFunction->getInput("ground truth"), "ground truth slices");

	// outputs
	registerOutput(_mergeTreeReader->getOutput("slices"), "slices");
	registerOutput(_mergeTreeReader->getOutput("conflict sets"), "conflict sets");
	if (createBestEffort) {

		registerOutput(_bestEffortReconstructor->getOutput(), "best effort slices");
		registerOutput(_bestEffortLossFunction->getOutput(), "best effort loss function");
	}
}

void
ReadMergeTreePipeline::updateOutputs() {}

boost::shared_ptr<pipeline::ProcessNode>
ReadMergeTreePipeline::getLoss(std::string name) {

	boost::shared_ptr<pipeline::ProcessNode> loss;

	if (name == "contourdistance") {

		LOG_USER(readmergetreepipelinelog) << "using slice loss ContourDistanceLoss" << std::endl;
		loss = boost::make_shared<ContourDistanceLoss>();

		loss->setInput("slices", _mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", _gtSlices);

	} else if (name == "overlap") {

		LOG_USER(readmergetreepipelinelog) << "using slice loss OverlapLoss" << std::endl;
		loss = boost::make_shared<OverlapLoss>();

		loss->setInput("slices", _mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", _gtSlices);

	} else if (name == "slicedistance") {

		LOG_USER(readmergetreepipelinelog) << "using slice loss SliceDistanceLoss" << std::endl;
		loss = boost::make_shared<SliceDistanceLoss>();

		loss->setInput("slices", _mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", _gtSlices);

	} else {

		UTIL_THROW_EXCEPTION(
				UsageError,
				"unknown best effort loss '" << name << "'");
	}

	return loss;
}
