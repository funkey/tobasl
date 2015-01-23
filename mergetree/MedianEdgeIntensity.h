#ifndef MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__
#define MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__

#include <util/cont_map.hpp>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/graph_algorithms.hxx>

/**
 * An edge scoring function that returns the median intensity of the edge 
 * pixels.
 */
class MedianEdgeIntensity {

public:

	typedef vigra::GridGraph<2> GridGraphType;

	struct EdgeNumConverter {

		typedef GridGraphType::index_type TargetType;

		EdgeNumConverter(GridGraphType& gridGraph) :
			graph(gridGraph) {}

		TargetType operator()(const GridGraphType::Edge& edge) const {
			return graph.id(edge);
		}

		GridGraphType::Edge operator()(const TargetType& id) const {
			return graph.edgeFromId(id);
		}

		GridGraphType& graph;
	};

	typedef util::cont_map<GridGraphType::Edge, float, EdgeNumConverter> EdgeWeightsType;

	struct EdgeComp {

		EdgeComp(const EdgeWeightsType& weights_)
			: weights(weights_) {}

		bool operator()(
				const GridGraphType::Edge& a,
				const GridGraphType::Edge& b) const {
			return weights.at(a) < weights.at(b);
		}

		const EdgeWeightsType& weights;
	};

	MedianEdgeIntensity(const vigra::MultiArrayView<2, float> intensities) :
		_grid(intensities.shape()),
		_edgeWeights(EdgeNumConverter(_grid)) {

		vigra::edgeWeightsFromNodeWeights(
				_grid,
				intensities,
				_edgeWeights);

		if (_edgeWeights.size() == 0)
			return;
		_maxEdgeWeight = _edgeWeights.begin()->second;
		for (EdgeWeightsType::const_iterator i = _edgeWeights.begin(); i != _edgeWeights.end(); i++)
			_maxEdgeWeight = std::max(_maxEdgeWeight, i->second);
	}

	float operator()(std::vector<GridGraphType::Edge>& edge) const {

		std::vector<GridGraphType::Edge>::iterator median = edge.begin() + edge.size()/2;
		std::nth_element(edge.begin(), median, edge.end(), EdgeComp(_edgeWeights));

		return _edgeWeights.at(*median);
	}

private:

	GridGraphType                 _grid;
	//GridGraphType::EdgeMap<float> _edgeWeights;
	EdgeWeightsType               _edgeWeights;
	float                         _maxEdgeWeight;
};

#endif // MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__

