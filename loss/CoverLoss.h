#ifndef MULTI2CUT_LOSS_COVER_LOSS_H__
#define MULTI2CUT_LOSS_COVER_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/SlicesTree.h>
#include "LossFunction.h"

/**
 * A loss implementing the dot-cover score introduced in
 *
 *   Arteta et al, "Learning to Detect Cells Using Non-overlapping Extremal 
 *   Regions", MICCAI 2012.
 *
 * The loss is composed of two contributions:
 *
 *   1) the absolute number of dots covered by each selected candidate minus 
 *   one, i.e, 1 if no dot is covered, and number of covered dots - 1 else.
 *
 *   2) the number of not covered dots, i.e., the total number of dots, minus  
 *   the number of dots covered by any candidate
 *
 * Hence, the loss is
 *
 *   Δ(y',y) = N - Σ_i y_i*n_i + Σ_i y_i*|n_i - 1|
 *
 * where N is the total number of dots, and n_i the number of dots covered by 
 * candidate i. Hence
 *
 *   Δ(y',y) = N + Σ_i  y_i*(|n_i - 1| - n_i)
 *           = N + Σ_i -y_i*[n_i≥1]
 */
class CoverLoss : public pipeline::SimpleProcessNode<> {

public:

	CoverLoss();

private:

	void updateOutputs();

	pipeline::Input<SlicesTree>    _slices;
	pipeline::Input<Slices>        _groundTruth;
	pipeline::Output<LossFunction> _lossFunction;

	double _constant;

	vigra::MultiArray<2, bool> _centroids;
};

#endif // MULTI2CUT_LOSS_COVER_LOSS_H__

