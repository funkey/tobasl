#ifndef MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__
#define MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__

#include <util/cont_map.hpp>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/graph_algorithms.hxx>
#include "EdgeNumConverter.h"

/**
 * An edge scoring function that returns the median intensity of the edge 
 * pixels.
 */
class MedianEdgeIntensity {

public:

	typedef vigra::GridGraph<2> GridGraphType;

	typedef GridGraphType::EdgeMap<float> EdgeWeightsType;

	struct EdgeComp {

		EdgeComp(const EdgeWeightsType& weights_)
			: weights(weights_) {}

		bool operator()(
				const GridGraphType::Edge& a,
				const GridGraphType::Edge& b) const {
			return weights[a] < weights[b];
		}

		const EdgeWeightsType& weights;
	};

	MedianEdgeIntensity(const vigra::MultiArrayView<2, float> intensities) :
		_grid(intensities.shape()),
		_edgeWeights(_grid) {

		vigra::edgeWeightsFromNodeWeights(
				_grid,
				intensities,
				_edgeWeights);

		if (_edgeWeights.size() == 0)
			return;
		_maxEdgeWeight = *_edgeWeights.begin();
		for (EdgeWeightsType::const_iterator i = _edgeWeights.begin(); i != _edgeWeights.end(); i++)
			_maxEdgeWeight = std::max(_maxEdgeWeight, *i);
	}

	float operator()(std::vector<GridGraphType::Edge>& edge) const {

		std::vector<GridGraphType::Edge>::iterator median = edge.begin() + edge.size()/2;
		std::nth_element(edge.begin(), median, edge.end(), EdgeComp(_edgeWeights));

		return _edgeWeights[*median];
	}

private:

	GridGraphType   _grid;
	EdgeWeightsType _edgeWeights;
	float           _maxEdgeWeight;
};

#endif // MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__

