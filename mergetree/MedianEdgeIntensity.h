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

	float operator()(const std::vector<GridGraphType::Edge>& edge) const {

		std::cout << "." << std::flush;

		std::vector<GridGraphType::Edge> sorted = edge;
		std::sort(
				sorted.begin(),
				sorted.end(),
				EdgeComp(_edgeWeights));

		return _edgeWeights.at(sorted[sorted.size()/2]);

		//assert(edge.size() > 0);

		//float median = _edgeWeights[edge.begin()];
		//unsigned int size   = edge.size();
		//unsigned int left   = 0;
		//unsigned int center = 0;
		//unsigned int right  = 0;

		////    *       size/2
		//// 123456789  i
		//// llllrrrrr  okay: |l| <= size/2, |r| <= size - size/2
		////
		////     *      size/2
		//// 1234567890 i
		//// lllcccrrrr
		//// lllllrrrrr
		//while (left > size/2 || right > size/2) {

			
		//}
	}

private:

	GridGraphType                 _grid;
	//GridGraphType::EdgeMap<float> _edgeWeights;
	EdgeWeightsType               _edgeWeights;
	float                         _maxEdgeWeight;
};

#endif // MULTI2CUT_MERGETREE_MEDIAN_EDGE_INTENSITY_H__

