#ifndef MULTI2CUT_LOSS_RAND_LOSS_H__
#define MULTI2CUT_LOSS_RAND_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/SlicesTree.h>
#include "LossFunction.h"

class RandLoss : public pipeline::SimpleProcessNode<> {

public:

	RandLoss();

private:

	void updateOutputs();

	void traverseAboveBestEffort(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort);

	void traverseBelowBestEffort(boost::shared_ptr<SlicesTree::Node> node);

	std::vector<size_t> getDescentantBestEffortSizes(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort);

	pipeline::Inputs<SlicesTree>   _slices;
	pipeline::Inputs<Slices>       _bestEffort;
	pipeline::Output<LossFunction> _lossFunction;

	double _weightSplit;
	double _weightMerge;
	double _weightFp;
	double _weightFn;

	double _constant;
};

#endif // MULTI2CUT_LOSS_RAND_LOSS_H__

