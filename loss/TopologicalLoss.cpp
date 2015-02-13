#include <util/Logger.h>
#include "TopologicalLoss.h"

logger::LogChannel topologicallosslog("topologicallosslog", "[TopologicalLoss] ");

TopologicalLoss::TopologicalLoss() {

	registerInputs(_slices, "slices");
	registerInputs(_bestEffort, "best effort");
	registerOutput(_lossFunction, "loss function");
}

void
TopologicalLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();

	for (unsigned int i = 0; i < _slices.size(); i++) {

		// get the plain split and merge costs
		foreach (boost::shared_ptr<SlicesTree::Node> root, _slices[i]->getRoots())
			traverse(root, *_bestEffort[i]);
	}

	// normalize them to be in [0,1)
	double topologicalUpperBound = 0;
	unsigned int id;
	double costs;
	foreach (boost::tie(id, costs), *_lossFunction)
		topologicalUpperBound += costs;
	foreach (boost::tie(id, costs), *_lossFunction)
		(*_lossFunction)[id] /= topologicalUpperBound;

	LOG_DEBUG(topologicallosslog)
			<< "upper bound on topological costs is "
			<< topologicalUpperBound << std::endl;

	// set a reward of -1 for the best-effort
	foreach (boost::shared_ptr<Slices> slices, _bestEffort) {

		foreach (boost::shared_ptr<Slice> slice, *slices)
			(*_lossFunction)[slice->getId()] = -1;
	}
}

double
TopologicalLoss::traverse(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(topologicallosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	bool isBestEffort = (std::find(bestEffort.begin(), bestEffort.end(), slice) != bestEffort.end());

	LOG_DEBUG(topologicallosslog) << "\tis best effort: " << isBestEffort << std::endl;

	if (isBestEffort) {

		assignSplitCosts(node, 0);
		return 0;
	}

	unsigned int numChildren = node->getChildren().size();

	LOG_DEBUG(topologicallosslog) << "\tthis slice has " << numChildren << " children" << std::endl;

	if (numChildren == 0) {

		// We are not below best-effort, and we don't have children -- this 
		// slice belongs to a path that is completely spurious.

		// give it merge costs of 1
		(*_lossFunction)[slice->getId()] = 1;
		return 1;
	}

	// we are above the best effort solution

	// sum the merge costs of the children
	double childrenMergeCosts = 0;
	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		childrenMergeCosts += traverse(child, bestEffort);

	// assign merge costs for the current node
	double mergeCosts = static_cast<double>(numChildren) - 1 + childrenMergeCosts;

	LOG_DEBUG(topologicallosslog) << "\tthis slice is above best-effort, assign merge costs of " << mergeCosts << std::endl;

	(*_lossFunction)[slice->getId()] = mergeCosts;

	return mergeCosts;
}

void
TopologicalLoss::assignSplitCosts(boost::shared_ptr<SlicesTree::Node> node, double costs) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(topologicallosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	(*_lossFunction)[slice->getId()] = costs;

	double k = node->getChildren().size();

	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		assignSplitCosts(child, costs + (k - 1)/k);
}
