#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "TopologicalLoss.h"

logger::LogChannel topologicallosslog("topologicallosslog", "[TopologicalLoss] ");

util::ProgramOption optionTopologicalLossWeightSplit(
		util::_module           = "loss.topological",
		util::_long_name        = "weightSplit",
		util::_description_text = "The weight of a split error in the topological loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionTopologicalLossWeightMerge(
		util::_module           = "loss.topological",
		util::_long_name        = "weightMerge",
		util::_description_text = "The weight of a merge error in the topological loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionTopologicalLossWeightFp(
		util::_module           = "loss.topological",
		util::_long_name        = "weightFp",
		util::_description_text = "The weight of a false positive error in the topological loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionTopologicalLossWeightFn(
		util::_module           = "loss.topological",
		util::_long_name        = "weightFn",
		util::_description_text = "The weight of a false negative error in the topological loss. Default is 1.0.",
		util::_default_value    = 1.0);

TopologicalLoss::TopologicalLoss() :
		_weightSplit(optionTopologicalLossWeightSplit),
		_weightMerge(optionTopologicalLossWeightMerge),
		_weightFp(optionTopologicalLossWeightFp),
		_weightFn(optionTopologicalLossWeightFn) {

	registerInputs(_slices, "slices");
	registerInputs(_bestEffort, "best effort");
	registerOutput(_lossFunction, "loss function");
}

void
TopologicalLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();
	_constant = 0;

	// for each slices tree
	for (unsigned int i = 0; i < _slices.size(); i++) {

		// get the topological costs
		foreach (boost::shared_ptr<SlicesTree::Node> root, _slices[i]->getRoots())
			traverseAboveBestEffort(root, *_bestEffort[i]);
	}


	// set the constant
	_lossFunction->setConstant(_constant);
}

TopologicalLoss::NodeCosts
TopologicalLoss::traverseAboveBestEffort(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(topologicallosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	bool isBestEffort = (std::find(bestEffort.begin(), bestEffort.end(), slice) != bestEffort.end());

	LOG_DEBUG(topologicallosslog) << "\tis best effort: " << isBestEffort << std::endl;

	if (isBestEffort) {

		NodeCosts bestEffortCosts;
		bestEffortCosts.split = 0;
		bestEffortCosts.merge = 0;
		bestEffortCosts.fp    = 0;
		bestEffortCosts.fn    = -_weightFn;
		_constant += _weightFn;

		// this assigns the cost to node and all descendants
		traverseBelowBestEffort(node, bestEffortCosts);

		return bestEffortCosts;
	}

	unsigned int numChildren = node->getChildren().size();

	LOG_DEBUG(topologicallosslog) << "\tthis slice has " << numChildren << " children" << std::endl;

	if (numChildren == 0) {

		// We are above best-effort, and we don't have children -- this slice 
		// belongs to a path that is completely spurious.

		// give it false positive costs
		NodeCosts falsePositiveCosts;
		falsePositiveCosts.split = 0;
		falsePositiveCosts.merge = 0;
		falsePositiveCosts.fp    = _weightFp;
		falsePositiveCosts.fn    = 0;

		(*_lossFunction)[slice->getId()] = falsePositiveCosts;

		return falsePositiveCosts;
	}

	// we are above the best effort solution

	// get our node costs from the costs of our children
	double sumChildMergeCosts = 0;
	double sumChildFnCosts    = 0;
	double minChildFpCosts    = std::numeric_limits<double>::infinity();
	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren()) {

		NodeCosts childCosts = traverseAboveBestEffort(child, bestEffort);

		sumChildMergeCosts += childCosts.merge;
		sumChildFnCosts    += childCosts.fn;
		minChildFpCosts     = std::min(minChildFpCosts, childCosts.fp);
	}

	NodeCosts costs;
	costs.split = 0;
	costs.merge = _weightMerge*(numChildren - 1) + sumChildMergeCosts;
	costs.fn    = sumChildFnCosts;
	costs.fp    = minChildFpCosts;

	LOG_DEBUG(topologicallosslog) << "\tthis slice is above best-effort, assign total costs of " << costs << std::endl;

	(*_lossFunction)[slice->getId()] = costs;

	return costs;
}

void
TopologicalLoss::traverseBelowBestEffort(boost::shared_ptr<SlicesTree::Node> node, NodeCosts costs) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	LOG_DEBUG(topologicallosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << slice->getComponent()->getSize()
			<< std::endl;

	(*_lossFunction)[slice->getId()] = costs;

	double k = node->getChildren().size();

	// get the children's costs
	NodeCosts childCosts;
	childCosts.split = costs.split + _weightSplit*(k - 1)/k;
	childCosts.merge = 0;
	childCosts.fn    = costs.fn/k;
	childCosts.fp    = 0;

	// propagate costs downwards
	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		traverseBelowBestEffort(child, childCosts);
}
