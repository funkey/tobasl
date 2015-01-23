#ifndef MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__
#define MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

#include <iostream>
#include <util/Logger.h>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/multi_watersheds.hxx>
#include <vigra/adjacency_list_graph.hxx>

extern logger::LogChannel mergetreelog;

class IterativeRegionMerging {

public:

	typedef vigra::GridGraph<2>       GridGraphType;
	typedef vigra::AdjacencyListGraph RagType;

	IterativeRegionMerging(vigra::MultiArrayView<2, int> initialRegions);

	template <typename ScoringFunction>
	void createMergeTree(const ScoringFunction& scoringFunction);

	vigra::MultiArrayView<2, int> getMergeTree() { return _mergeTree; }

private:

	// merge two regions and return new region node
	template <typename ScoringFunction>
	RagType::Node mergeRegions(
			RagType::Node regionA,
			RagType::Node regionB,
			const ScoringFunction& scoringFunction);

	void finishMergeTree();

	GridGraphType                 _grid;
	GridGraphType::EdgeMap<float> _gridEdgeWeights;

	RagType _rag;

	std::map<RagType::Edge, std::vector<GridGraphType::Edge> > _ragToGridEdges;
	std::map<RagType::Node, std::size_t>                       _regionSizes;
	std::map<RagType::Node, RagType::Node>                     _parentNodes;
	std::map<RagType::Edge, float>                             _edgeScores;

	vigra::MultiArray<2, int> _mergeTree;

	std::size_t _smallRegionThreshold;
};

template <typename ScoringFunction>
void
IterativeRegionMerging::createMergeTree(const ScoringFunction& scoringFunction) {

	LOG_USER(mergetreelog) << "computing initial edge scores..." << std::endl;

	// compute initial edge scores
	for (RagType::EdgeIt edge(_rag); edge != lemon::INVALID; ++edge)
		_edgeScores[*edge] = scoringFunction(_ragToGridEdges[*edge]);

	LOG_USER(mergetreelog) << "merging small regions first..." << std::endl;

	// get all small regions
	std::vector<RagType::Node> smallRegions;
	for (RagType::NodeIt node(_rag); node != lemon::INVALID; ++node)
		if (_regionSizes[*node] <= _smallRegionThreshold)
			smallRegions.push_back(*node);

	// merge small regions
	bool done = false;
	while (!done) {

		done = true;

		for (std::vector<RagType::Node>::const_iterator node = smallRegions.begin(); node != smallRegions.end(); node++) {

			// skip already merged nodes
			if (_parentNodes[*node] != lemon::INVALID)
				continue;

			// find lowest edge
			RagType::Edge minEdge;
			float         minScore = 0;
			for (RagType::IncEdgeIt edge(_rag, *node); edge != lemon::INVALID; ++edge) {

				// skip already merged neighbors
				if (
						(*node == _rag.u(*edge) && _parentNodes[_rag.v(*edge)] != lemon::INVALID) ||
						(*node == _rag.v(*edge) && _parentNodes[_rag.u(*edge)] != lemon::INVALID))
					continue;

				if (minEdge == lemon::INVALID || _edgeScores[*edge] < minScore) {

					minEdge  = *edge;
					minScore = _edgeScores[*edge];
				}
			}

			// no valid neighbor
			if (minEdge == lemon::INVALID)
				continue;

			RagType::Node u = _rag.u(minEdge);
			RagType::Node v = _rag.v(minEdge);

			RagType::Node merged = mergeRegions(u, v, scoringFunction);

			LOG_DEBUG(mergetreelog)
					<< "merged small regions " << _rag.id(u) << " and " << _rag.id(v)
					<< " with score " << minScore
					<< " into " << _rag.id(merged) << std::endl;

			done = false;
		}
	}

	LOG_USER(mergetreelog) << "merging other regions..." << std::endl;

	// TODO...

	LOG_USER(mergetreelog) << "finishing merge tree image..." << std::endl;

	finishMergeTree();
}

template <typename ScoringFunction>
IterativeRegionMerging::RagType::Node
IterativeRegionMerging::mergeRegions(
		RagType::Node a,
		RagType::Node b,
		const ScoringFunction& scoringFunction) {

	// don't merge previously merged nodes
	assert(_parentNodes[a] == lemon::INVALID);
	assert(_parentNodes[b] == lemon::INVALID);

	RagType::Edge edge = _rag.findEdge(a, b);

	if (edge == lemon::INVALID)
		return RagType::Node();

	// add new c = a + b
	RagType::Node c = _rag.addNode();

	// label the edge pixels between a and b with c
	for (std::vector<GridGraphType::Edge>::iterator i = _ragToGridEdges[edge].begin();
	     i != _ragToGridEdges[edge].end(); i++)
		 _mergeTree[std::min(_grid.u(*i), _grid.v(*i))] = _rag.id(c);

	_parentNodes[a] = c;
	_parentNodes[b] = c;

	// connect c to neighbors of a and b and set affiliated edges accordingly

	std::vector<RagType::Edge> newEdges;

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

			newEdges.push_back(newEdge);
		}
	}

	// get edge score for new edges
	for (std::vector<RagType::Edge>::const_iterator i = newEdges.begin(); i != newEdges.end(); i++)
		_edgeScores[*i] = scoringFunction(_ragToGridEdges[*i]);

	return c;
}

#endif // MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

