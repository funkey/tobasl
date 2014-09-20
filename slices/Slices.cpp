#include "Slices.h"

Slices::Slices() :
	_adaptor(0),
	_kdTree(0),
	_kdTreeDirty(true) {}

Slices::Slices(const Slices& other) :
	pipeline::Data(),
	_slices(other._slices),
	_adaptor(0),
	_kdTree(0),
	_kdTreeDirty(true) {}

Slices&
Slices::operator=(const Slices& other) {

	if (_kdTree)
		delete _kdTree;

	if (_adaptor)
		delete _adaptor;

	_kdTree = 0;
	_adaptor = 0;
	_kdTreeDirty = true;

	_slices = other._slices;

	return *this;
}

Slices::~Slices() {

	if (_kdTree)
		delete _kdTree;

	if (_adaptor)
		delete _adaptor;
}

void
Slices::clear() {

	_slices.clear();
}

void
Slices::add(boost::shared_ptr<Slice> slice) {

	_slices.push_back(slice);

	_kdTreeDirty = true;
}

void
Slices::addAll(const Slices& slices) {

	_slices.insert(_slices.end(), slices.begin(), slices.end());

	_kdTreeDirty = true;
}

void
Slices::remove(boost::shared_ptr<Slice> slice) {

	for (unsigned int i = 0; i < _slices.size(); i++)
		if (_slices[i] == slice) {

			_slices.erase(_slices.begin() + i);
			return;
		}

	return;
}

std::vector<boost::shared_ptr<Slice> >
Slices::find(const util::point<double>& center, double distance) {

	if (_kdTreeDirty) {

		delete _adaptor;
		delete _kdTree;

		_adaptor = 0;
		_kdTree = 0;
	}

	// create kd-tree, if it does not exist
	if (!_kdTree) {

		// create slice vector adaptor
		_adaptor = new SliceVectorAdaptor(_slices);

		// create the tree
		_kdTree = new SliceKdTree(2, *_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(10));

		// create index
		_kdTree->buildIndex();
	}

	// find close indices
	std::vector<std::pair<size_t, double> > results;

	double query[2];
	query[0] = center.x;
	query[1] = center.y;

	nanoflann::SearchParams params(0 /* ignored parameter */);

	_kdTree->radiusSearch(&query[0], distance, results, params);

	// fill result vector
	size_t index;
	double dist;

	std::vector<boost::shared_ptr<Slice> > found;

	foreach (boost::tie(index, dist), results)
		found.push_back(_slices[index]);

	return found;
}

void
Slices::translate(const util::point<int>& offset) {

	foreach (boost::shared_ptr<Slice> slice, _slices)
		slice->translate(offset);

	_kdTreeDirty = true;
}
