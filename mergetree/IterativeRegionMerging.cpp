#include <cassert>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <vigra/graph_algorithms.hxx>
#include "IterativeRegionMerging.h"

util::ProgramOption optionSmallRegionThreshold(
		util::_long_name        = "smallRegionThreshold",
		util::_description_text = "Maximal size of a region to be considered small. Small regions are merged in a first pass before others are considered.",
		util::_default_value    = 100);

logger::LogChannel mergetreelog("mergetreelog", "[IterativeRegionMerging] ");

IterativeRegionMerging::IterativeRegionMerging(
		vigra::MultiArrayView<2, int> initialRegions) :
	_grid(initialRegions.shape()),
	_gridEdgeWeights(_grid),
	_regions(_grid),
	_mergeTree(initialRegions.shape()) {

	_regions = initialRegions;

	// get initial region adjecancy graph
	RagType::EdgeMap<std::vector<GridGraphType::Edge> > affiliatedEdges;
	vigra::makeRegionAdjacencyGraph(
			_grid,
			_regions,
			_rag,
			affiliatedEdges);

	// get region sizes
	for (vigra::MultiArray<2, int>::iterator i = _regions.begin(); i != _regions.end(); i++)
		_regionSizes[_rag.nodeFromId(*i)]++;

	for (RagType::EdgeIt edge(_rag); edge != lemon::INVALID; ++edge)
		_ragToGridEdges[*edge] = affiliatedEdges[*edge];

	_smallRegionThreshold = optionSmallRegionThreshold;

	int numRegions = 0;
	for (RagType::NodeIt node(_rag); node != lemon::INVALID; ++node)
		numRegions++;
	int numRegionEdges = _ragToGridEdges.size();

	LOG_USER(mergetreelog)
			<< "got region adjecancy graph with "
			<< numRegions << " regions and "
			<< numRegionEdges << " edges" << std::endl;
}
