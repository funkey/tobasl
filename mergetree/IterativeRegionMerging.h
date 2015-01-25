#ifndef MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__
#define MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

#include <iostream>
#include <util/Logger.h>
#include <util/cont_map.hpp>
#include <util/assert.h>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/multi_watersheds.hxx>
#include <vigra/adjacency_list_graph.hxx>
#include "NodeNumConverter.h"
#include "EdgeNumConverter.h"

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

	typedef util::cont_map<RagType::Edge, std::vector<GridGraphType::Edge>, EdgeNumConverter<RagType> > GridEdgesType;
	typedef util::cont_map<RagType::Node, std::size_t, NodeNumConverter<RagType> >                      RegionSizesType;
	typedef util::cont_map<RagType::Node, RagType::Node, NodeNumConverter<RagType> >                    ParentNodesType;
	typedef util::cont_map<RagType::Edge, float, EdgeNumConverter<RagType> >                            EdgeScoresType;

	struct EdgeCompare {

		EdgeCompare(const EdgeScoresType& edgeScores_) :
			edgeScores(edgeScores_) {}

		bool operator()(const RagType::Edge& a, const RagType::Edge& b) {

			UTIL_ASSERT(edgeScores.count(a));
			UTIL_ASSERT(edgeScores.count(b));

			// sort edges in increasing score value
			return edgeScores[a] > edgeScores[b];
		}

		const EdgeScoresType& edgeScores;
	};

	typedef std::priority_queue<RagType::Edge, std::vector<RagType::Edge>, EdgeCompare> MergeEdgesType;

	// merge two regions and return new region node
	template <typename ScoringFunction>
	RagType::Node mergeRegions(
			RagType::Node regionA,
			RagType::Node regionB,
			const ScoringFunction& scoringFunction);

	// merge two regions identified by an edge
	// a and b can optionally be given as u and v of the edge to avoid a lookup
	template <typename ScoringFunction>
	RagType::Node mergeRegions(
			RagType::Edge edge,
			const ScoringFunction& scoringFunction,
			RagType::Node a = RagType::Node(),
			RagType::Node b = RagType::Node());

	template <typename ScoringFunction>
	void scoreEdge(const RagType::Edge& edge, const ScoringFunction& scoringFunction);

	inline RagType::Edge nextMergeEdge() { float _; return nextMergeEdge(_); }
	inline RagType::Edge nextMergeEdge(float& score);

	void finishMergeTree();

	GridGraphType                 _grid;
	GridGraphType::EdgeMap<float> _gridEdgeWeights;

	RagType _rag;

	GridEdgesType   _ragToGridEdges;
	RegionSizesType _regionSizes;
	ParentNodesType _parentNodes;
	EdgeScoresType  _edgeScores;

	vigra::MultiArray<2, int> _mergeTree;

	std::size_t _smallRegionThreshold;

	MergeEdgesType _mergeEdges;
};

template <typename ScoringFunction>
void
IterativeRegionMerging::createMergeTree(const ScoringFunction& scoringFunction) {

	LOG_USER(mergetreelog) << "computing initial edge scores..." << std::endl;

	// compute initial edge scores
	for (RagType::EdgeIt edge(_rag); edge != lemon::INVALID; ++edge)
		scoreEdge(*edge, scoringFunction);

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

	RagType::Edge next;
	float         score;

	while (true) {

		next = nextMergeEdge(score);
		if (next == lemon::INVALID)
			break;

		RagType::Node merged = mergeRegions(next, scoringFunction);

		LOG_ALL(mergetreelog)
				<< "merged regions " << _rag.id(_rag.u(next)) << " and " << _rag.id(_rag.v(next))
				<< " with score " << score
				<< " into " << _rag.id(merged) << std::endl;
	}

	finishMergeTree();
}

template <typename ScoringFunction>
IterativeRegionMerging::RagType::Node
IterativeRegionMerging::mergeRegions(
		RagType::Node a,
		RagType::Node b,
		const ScoringFunction& scoringFunction) {

	// don't merge previously merged nodes
	UTIL_ASSERT(_parentNodes[a] == lemon::INVALID);
	UTIL_ASSERT(_parentNodes[b] == lemon::INVALID);

	RagType::Edge edge = _rag.findEdge(a, b);

	if (edge == lemon::INVALID)
		return RagType::Node();

	return mergeRegions(edge, scoringFunction, a, b);
}

template <typename ScoringFunction>
IterativeRegionMerging::RagType::Node
IterativeRegionMerging::mergeRegions(
		RagType::Edge edge,
		const ScoringFunction& scoringFunction,
		RagType::Node a,
		RagType::Node b) {

	if (a == lemon::INVALID || b == lemon::INVALID) {

		a = _rag.u(edge);
		b = _rag.v(edge);
	}

	// don't merge previously merged nodes
	UTIL_ASSERT(_parentNodes[a] == lemon::INVALID);
	UTIL_ASSERT(_parentNodes[b] == lemon::INVALID);

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
		scoreEdge(*i, scoringFunction);

	return c;
}

template <typename ScoringFunction>
void
IterativeRegionMerging::scoreEdge(const RagType::Edge& edge, const ScoringFunction& scoringFunction) {

	_edgeScores[edge] = scoringFunction(_ragToGridEdges[edge]);
	_mergeEdges.push(edge);
}

IterativeRegionMerging::RagType::Edge
IterativeRegionMerging::nextMergeEdge(float& score) {

	RagType::Edge next;

	while (true) {

		// no more edges
		if (_mergeEdges.size() == 0)
			return RagType::Edge();

		next = _mergeEdges.top(); _mergeEdges.pop();

		// don't accept edges to already merged regions
		if (_parentNodes[_rag.u(next)] != lemon::INVALID || _parentNodes[_rag.v(next)] != lemon::INVALID)
			continue;

		break;
	}

	score = _edgeScores[next];

	return next;
}

#endif // MULTI2CUT_MERGETREE_ITERATIVE_REGION_MERGING_H__

