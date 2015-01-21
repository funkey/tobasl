#ifndef MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__
#define MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

#include <iostream>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/multi_watersheds.hxx>
#include <vigra/graph_algorithms.hxx>

class IterativeRegionMerging {

public:

	typedef vigra::GridGraph<2>       GridGraphType;
	typedef vigra::AdjacencyListGraph RagType;

	IterativeRegionMerging(vigra::MultiArrayView<2, int> initialRegions);

	void createMergeTree();

	const GridGraphType::NodeMap<int>& getRegions() const { return _regions; }

private:

	// merge two regions and return new region node
	RagType::Node mergeRegions(
			RagType::Node regionA,
			RagType::Node regionB);

	GridGraphType                 _grid;
	GridGraphType::EdgeMap<float> _gridEdgeWeights;
	GridGraphType::NodeMap<int>   _regions;

	RagType _rag;

	std::map<RagType::Edge, std::vector<GridGraphType::Edge> > _ragToGridEdges;
	std::map<RagType::Node, std::size_t>                       _regionSizes;
	std::map<RagType::Node, RagType::Node>                     _parentNodes;

	vigra::MultiArray<2, int> _mergeTree;

};

#endif // MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

