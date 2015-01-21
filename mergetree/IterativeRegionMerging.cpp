#include <cassert>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "IterativeRegionMerging.h"

util::ProgramOption optionSmallRegionThreshold(
		util::_long_name        = "smallRegionThreshold",
		util::_description_text = "Maximal size of a region to be considered small. Small regions are merged in a first pass before others are considered..",
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

	int numRegions = 0;
	for (RagType::NodeIt node(_rag); node != lemon::INVALID; ++node)
		numRegions++;
	int numRegionEdges = _ragToGridEdges.size();

	LOG_USER(mergetreelog)
			<< "got region adjecancy graph with "
			<< numRegions << " regions and "
			<< numRegionEdges << " edges" << std::endl;

	// get region sizes
	for (vigra::MultiArray<2, int>::iterator i = _regions.begin(); i != _regions.end(); i++)
		_regionSizes[_rag.nodeFromId(*i)]++;

	for (RagType::EdgeIt edge(_rag); edge != lemon::INVALID; ++edge)
		_ragToGridEdges[*edge] = affiliatedEdges[*edge];
}

void
IterativeRegionMerging::createMergeTree() {

	// prepare mergetree image
	_mergeTree = 0;

	std::size_t smallRegionThreshold = optionSmallRegionThreshold;

	LOG_USER(mergetreelog) << "merging small regions first..." << std::endl;

	bool done = false;
	while (!done) {

		done = true;

		for (RagType::EdgeIt edge(_rag); edge != lemon::INVALID; ++edge) {

			RagType::Node u = _rag.u(*edge);
			RagType::Node v = _rag.v(*edge);

			assert(u != v);

			// skip already merged nodes
			if (_parentNodes[u] != lemon::INVALID || _parentNodes[v] != lemon::INVALID)
				continue;

			LOG_ALL(mergetreelog) << "probing regions " << _rag.id(u) << " and " << _rag.id(v) << std::endl;

			if (_regionSizes[u] < smallRegionThreshold || _regionSizes[v] < smallRegionThreshold) {

				LOG_DEBUG(mergetreelog) << "merging small regions " << _rag.id(u) << " and " << _rag.id(v);
				RagType::Node merged = mergeRegions(u, v);
				LOG_DEBUG(mergetreelog) << " into " << _rag.id(merged) << std::endl;

				done = false;
				break;
			}
		}
	}

	LOG_USER(mergetreelog) << "merging other regions..." << std::endl;
}

IterativeRegionMerging::RagType::Node
IterativeRegionMerging::mergeRegions(
		RagType::Node a,
		RagType::Node b) {

	// don't merge previously merged nodes
	assert(_parentNodes[a] == lemon::INVALID);
	assert(_parentNodes[b] == lemon::INVALID);

	RagType::Edge edge = _rag.findEdge(a, b);

	if (edge == lemon::INVALID)
		return RagType::Node();

	// add new c = a + b
	RagType::Node c = _rag.addNode();

	_parentNodes[a] = c;
	_parentNodes[b] = c;

	// connect c to neighbors of a and b and set affiliated edges accordingly

	// for child in {a,b}
	for (int i = 0; i <= 1; i++) {
		RagType::Node child = (i == 0 ? a : b);
		RagType::Node other = (i == 0 ? b : a);

		std::vector<RagType::Node> neighbors;
		std::vector<RagType::Edge> neighborEdges;

		// for all edges incident to child
		for (RagType::IncEdgeIt edge(_rag, child); edge != lemon::INVALID; ++edge) {

			// get the neighbor
			RagType::Node neighbor = (_rag.u(*edge) == child ? _rag.v(*edge) : _rag.u(*edge));

			if (neighbor == other)
				continue;

			neighbors.push_back(neighbor);
			neighborEdges.push_back(*edge);
		}

		// for all neighbors
		for (unsigned int i = 0; i < neighbors.size(); i++) {

			const RagType::Node neighbor     = neighbors[i];
			const RagType::Edge neighborEdge = neighborEdges[i];

			// add the edge from c->neighbor
			RagType::Edge newEdge = _rag.findEdge(c, neighbor);
			if (newEdge == lemon::INVALID)
				newEdge = _rag.addEdge(c, neighbor);

			// add affiliated edges from child->neighbor to new edge c->neighbor
			std::copy(
					_ragToGridEdges[neighborEdge].begin(),
					_ragToGridEdges[neighborEdge].end(),
					std::back_inserter(_ragToGridEdges[newEdge]));
		}
	}

	return c;
}
