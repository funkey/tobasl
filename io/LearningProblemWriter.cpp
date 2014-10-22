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

	getSliceCosts();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		costFunctionFile
				<< "var" << sliceVariableMap.getVariableNum(slice->getId()) << " "
				<< _costs[slice->getId()] << std::endl;
	}
}

void
LearningProblemWriter::getSliceCosts() {

	// get the plain split and merge costs
	foreach (boost::shared_ptr<SlicesTree::Node> root, _slices->getRoots())
		traverse(root);

	// normalize them to be in [0,1)
	double topologicalUpperBound = 0;
	unsigned int id;
	double costs;
	foreach (boost::tie(id, costs), _costs)
		topologicalUpperBound += costs;
	foreach (boost::tie(id, costs), _costs)
		_costs[id] /= topologicalUpperBound;

	LOG_DEBUG(learningproblemwriterlog)
			<< "upper bound on topological costs is "
			<< topologicalUpperBound << std::endl;

	// set a reward of -1 for the best-effort
	foreach (boost::shared_ptr<Slice> slice, *_bestEffort)
		_costs[slice->getId()] = -1;
}

double
LearningProblemWriter::traverse(boost::shared_ptr<SlicesTree::Node> node) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(learningproblemwriterlog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	bool isBestEffort = (std::find(_bestEffort->begin(), _bestEffort->end(), slice) != _bestEffort->end());

	LOG_DEBUG(learningproblemwriterlog) << "\tis best effort: " << isBestEffort << std::endl;

	if (isBestEffort) {

		assignSplitCosts(node, 0);
		return 0;
	}

	unsigned int numChildren = node->getChildren().size();

	LOG_DEBUG(learningproblemwriterlog) << "\tthis slice has " << numChildren << " children" << std::endl;

	if (numChildren == 0) {

		// We are not below best-effort, and we don't have children -- this 
		// slice belongs to a path that is completely spurious.

		// give it merge costs of 1
		_costs[slice->getId()] = 1;
		return 1;
	}

	// we are above the best effort solution

	// sum the merge costs of the children
	double childrenMergeCosts = 0;
	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		childrenMergeCosts += traverse(child);

	// assign merge costs for the current node
	double mergeCosts = static_cast<double>(numChildren) - 1 + childrenMergeCosts;

	LOG_DEBUG(learningproblemwriterlog) << "\tthis slice is above best-effort, assign merge costs of " << mergeCosts << std::endl;

	_costs[slice->getId()] = mergeCosts;

	return mergeCosts;
}

void
LearningProblemWriter::assignSplitCosts(boost::shared_ptr<SlicesTree::Node> node, double costs) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(learningproblemwriterlog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	_costs[slice->getId()] = costs;

	double k = node->getChildren().size();

	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		assignSplitCosts(child, costs + (k - 1)/k);
}
