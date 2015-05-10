#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/helpers.hpp>
#include "RandLoss.h"

logger::LogChannel randlosslog("randlosslog", "[RandLoss] ");

util::ProgramOption optionRandLossWeightSplit(
		util::_module           = "loss.rand",
		util::_long_name        = "weightSplit",
		util::_description_text = "The weight of a split error in the rand loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionRandLossWeightMerge(
		util::_module           = "loss.rand",
		util::_long_name        = "weightMerge",
		util::_description_text = "The weight of a merge error in the rand loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionRandLossWeightFp(
		util::_module           = "loss.rand",
		util::_long_name        = "weightFp",
		util::_description_text = "The weight of a false positive error in the rand loss. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionRandLossWeightFn(
		util::_module           = "loss.rand",
		util::_long_name        = "weightFn",
		util::_description_text = "The weight of a false negative error in the rand loss. Default is 1.0.",
		util::_default_value    = 1.0);

RandLoss::RandLoss() :
		_weightSplit(optionRandLossWeightSplit),
		_weightMerge(optionRandLossWeightMerge),
		_weightFp(optionRandLossWeightFp),
		_weightFn(optionRandLossWeightFn) {

	registerInputs(_slices, "slices");
	registerInputs(_bestEffort, "best effort");
	registerOutput(_lossFunction, "loss function");
}

void
RandLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();
	_constant = 0;

	// for each slices tree
	for (unsigned int i = 0; i < _slices.size(); i++) {

		// get the rand costs
		foreach (boost::shared_ptr<SlicesTree::Node> root, _slices[i]->getRoots())
			traverseAboveBestEffort(root, *_bestEffort[i]);
	}

	// get the negative cost of the best effort solution
	for (unsigned int i = 0; i < _slices.size(); i++) {
		foreach (boost::shared_ptr<Slice> slice, *_bestEffort[i])
			_constant -= (*_lossFunction)[slice->getId()];
	}

	// set the constant, such that the best effort has cost of 0
	_lossFunction->setConstant(_constant);
}

void
RandLoss::traverseAboveBestEffort(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	double size = slice->getComponent()->getSize();

	LOG_DEBUG(randlosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << size
			<< std::endl;

	bool isBestEffort = (std::find(bestEffort.begin(), bestEffort.end(), slice) != bestEffort.end());

	LOG_DEBUG(randlosslog) << "\tis best effort: " << isBestEffort << std::endl;

	if (isBestEffort) {

		// this assigns the cost to node and all descendants
		traverseBelowBestEffort(node);

		return;
	}

	std::vector<size_t> descendantBestEffortSizes = getDescentantBestEffortSizes(node, bestEffort);

	LOG_DEBUG(randlosslog) << "\tthis slice has " << descendantBestEffortSizes.size() << " best-effort descendants" << std::endl;
	LOG_DEBUG(randlosslog) << "\twith sizes " << descendantBestEffortSizes << std::endl;

	// we are above the best effort solution

	double costs = 0;
	for (int i = 0; i < descendantBestEffortSizes.size(); i++) {

		double a = descendantBestEffortSizes[i];

		costs += a*(a-1)/2;

		for (int j = i + 1; j < descendantBestEffortSizes.size(); j++) {

			double b = descendantBestEffortSizes[j];
			costs -= a*b;
		}
	}

	LOG_DEBUG(randlosslog) << "\tthis slice is above best-effort, assign total costs of " << costs << std::endl;

	(*_lossFunction)[slice->getId()] = -costs;

	// visit children

	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren()) {

		traverseAboveBestEffort(child, bestEffort);
	}
}

void
RandLoss::traverseBelowBestEffort(boost::shared_ptr<SlicesTree::Node> node) {

	boost::shared_ptr<Slice> slice = node->getSlice();

	double size = slice->getComponent()->getSize();

	LOG_DEBUG(randlosslog)
			<< "entering slice " << slice->getId()
			<< ", size = " << size
			<< std::endl;

	double costs = size*(size - 1)/2;
	(*_lossFunction)[slice->getId()] = -costs;

	foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren())
		traverseBelowBestEffort(child);
}

std::vector<size_t>
RandLoss::getDescentantBestEffortSizes(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort) {

	boost::shared_ptr<Slice> slice = node->getSlice();
	bool isBestEffort = (std::find(bestEffort.begin(), bestEffort.end(), slice) != bestEffort.end());

	std::vector<size_t> sizes;

	if (isBestEffort) {

		sizes.push_back(slice->getComponent()->getSize());

	} else {

		foreach (boost::shared_ptr<SlicesTree::Node> child, node->getChildren()) {

			std::vector<size_t> childSizes = getDescentantBestEffortSizes(child, bestEffort);
			std::copy(childSizes.begin(), childSizes.end(), std::back_inserter(sizes));
		}
	}

	return sizes;
}
