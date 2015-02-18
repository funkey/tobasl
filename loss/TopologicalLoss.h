#ifndef MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__
#define MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/SlicesTree.h>
#include "LossFunction.h"

class TopologicalLoss : public pipeline::SimpleProcessNode<> {

public:

	TopologicalLoss();

private:

	struct NodeCosts {

		double split;
		double merge;
		double fp;
		double fn;

		operator double() const {

			return split + merge + fp + fn;
		}
	};

	void updateOutputs();

	NodeCosts traverseAboveBestEffort(boost::shared_ptr<SlicesTree::Node> node, const Slices& bestEffort);

	void traverseBelowBestEffort(boost::shared_ptr<SlicesTree::Node> node, NodeCosts costs);

	pipeline::Inputs<SlicesTree>   _slices;
	pipeline::Inputs<Slices>       _bestEffort;
	pipeline::Output<LossFunction> _lossFunction;

	double _weightSplit;
	double _weightMerge;
	double _weightFp;
	double _weightFn;

	double _constant;
};

#endif // MULTI2CUT_LOSS_TOPOLOGICAL_LOSS_H__

