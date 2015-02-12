#ifndef MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__
#define MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/SlicesTree.h>
#include "LossFunction.h"

class TopologicalLoss : public pipeline::SimpleProcessNode<> {

public:

	TopologicalLoss();

private:

	void updateOutputs();

	double traverse(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort);

	void assignSplitCosts(boost::shared_ptr<SlicesTree::Node> node, double costs);

	pipeline::Inputs<SlicesTree>   _slices;
	pipeline::Inputs<Slices>       _bestEffort;
	pipeline::Output<LossFunction> _lossFunction;
};

#endif // MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__

