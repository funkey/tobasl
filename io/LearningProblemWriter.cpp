#include <fstream>
#include <inference/SliceVariableMap.h>
#include "LearningProblemWriter.h"
#include <util/Logger.h>

logger::LogChannel learningproblemwriterlog("learningproblemwriterlog", "[LearningProblemWriter] ");

LearningProblemWriter::LearningProblemWriter() {

	registerInput(_slices, "slices");
	registerInput(_conflictSets, "conflict sets");
	registerInput(_features, "features");
	registerInput(_bestEffort, "best effort");
}

void
LearningProblemWriter::write() {

	updateInputs();

	LOG_USER(learningproblemwriterlog) << "writing learning problem" << std::endl;

	std::ofstream featuresFile("features.txt");

	SliceVariableMap sliceVariableMap;
	unsigned int nextVarNum = 0;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		sliceVariableMap.associate(slice->getId(), nextVarNum);
		nextVarNum++;

		const std::vector<double>& features = _features->getFeatures(slice->getId());

		foreach (double f, features) {

			featuresFile << f << " ";
		}

		featuresFile << std::endl;
	}

	std::ofstream featuresMinMaxFile("features_minmax.txt");

	const std::vector<double>& min = _features->getMin();
	const std::vector<double>& max = _features->getMax();

	foreach (double f, min)
		featuresMinMaxFile << f << " ";
	featuresMinMaxFile << std::endl;
	foreach (double f, max)
		featuresMinMaxFile << f << " ";
	featuresMinMaxFile << std::endl;

	std::ofstream constraintsFile("constraints.txt");

	foreach (ConflictSet& conflictSet, *_conflictSets) {

		foreach (unsigned int sliceId, conflictSet.getSlices()) {

			unsigned int varNum = sliceVariableMap.getVariableNum(sliceId);

			constraintsFile << "1*" << varNum << " ";
		}

		constraintsFile << "<= 1" << std::endl;
	}

	std::ofstream labelsFile("labels.txt");

	std::set<unsigned int> bestEffortSliceIds;
	foreach (boost::shared_ptr<Slice> slice, *_bestEffort)
		bestEffortSliceIds.insert(slice->getId());

	for (unsigned int varNum = 0; varNum < nextVarNum; varNum++) {

		unsigned int sliceId = sliceVariableMap.getSliceId(varNum);

		if (bestEffortSliceIds.count(sliceId))
			labelsFile << 1 << std::endl;
		else
			labelsFile << 0 << std::endl;
	}

	std::ofstream costFunctionFile("cost_function.txt");

	costFunctionFile << "numVar = " << nextVarNum << std::endl;

	std::map<unsigned int, double> sliceCosts = getSliceCosts();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		costFunctionFile
				<< "var" << sliceVariableMap.getVariableNum(slice->getId()) << " "
				<< sliceCosts[slice->getId()] << std::endl;
	}
}

std::map<unsigned int, double>
LearningProblemWriter::getSliceCosts() {

	std::map<unsigned, double> sliceCosts;

	foreach (boost::shared_ptr<SlicesTree::Node> root, _slices->getRoots()) {

		traverse(root, sliceCosts, true /* above best-effort */);
	}

	return sliceCosts;
}

unsigned int
LearningProblemWriter::traverse(boost::shared_ptr<SlicesTree::Node> node, std::map<unsigned int, double>& costs, bool aboveBestEffort) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(learningproblemwriterlog) << "entering slice " << slice->getId() << ", size = " << slice->getComponent()->getSize() << std::endl;

	bool isBestEffort = (std::find(_bestEffort->begin(), _bestEffort->end(), slice) != _bestEffort->end());

	LOG_DEBUG(learningproblemwriterlog) << "\tis best effort: " << isBestEffort << std::endl;

	if (isBestEffort) {

		aboveBestEffort = false;
		costs[node->getSlice()->getId()] = 0;
	}

	unsigned int numChildren = node->getChildren().size();

	LOG_DEBUG(learningproblemwriterlog) << "\tthis slice has " << numChildren << " children" << std::endl;

	// assign split costs for each child
	if (!aboveBestEffort) {

		foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
			costs[child->getSlice()->getId()] = 1.0/numChildren;

		if (numChildren > 0)
			LOG_DEBUG(learningproblemwriterlog) << "\tthis slice is not above best-effort, assign children split costs of " << (1.0/numChildren) << std::endl;
	}

	LOG_DEBUG(learningproblemwriterlog) << "processing children" << std::endl;

	unsigned int numMerges = 0;
	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		numMerges += traverse(child, costs, aboveBestEffort);

	LOG_DEBUG(learningproblemwriterlog) << "done processing children" << std::endl;

	// assign merge costs for the current node
	if (aboveBestEffort) {

		LOG_DEBUG(learningproblemwriterlog) << "\tthis slice is above best-effort, assign merge costs of " << (numMerges + 1) << std::endl;

		costs[slice->getId()] = numMerges + 1;
		return numMerges + 1;
	}

	return 0;
}
