#ifndef CELLTRACKER_CELLS_H__
#define CELLTRACKER_CELLS_H__

#include <boost/shared_ptr.hpp>

#include <external/nanoflann/nanoflann.hpp>

#include <imageprocessing/ConnectedComponent.h>
#include <pipeline/all.h>
#include "Slice.h"

/**
 * An adaptor class to use std::vector<boost::shared_ptr<Slice> > in a
 * nanoflann kd-tree.
 */
class SliceVectorAdaptor {

	typedef std::vector<boost::shared_ptr<Slice> > slices_type;

public:

	/**
	 * Create a new adaptor.
	 */
	SliceVectorAdaptor(const slices_type& slices) :
		_slices(slices) {}

	/**
	 * Nanoflann access interface. Gets the number of data points.
	 */
	size_t kdtree_get_point_count() const { return _slices.size(); }

	/**
	 * Nanoflann access interface. Gets the distance between two data points.
	 */
	inline double kdtree_distance(const double *p1, const size_t index_p2, size_t) const {

		double d0 = p1[0] - _slices[index_p2]->getComponent()->getCenter().x;
		double d1 = p1[1] - _slices[index_p2]->getComponent()->getCenter().y;

		return d0*d0 + d1*d1;
	}

	/**
	 * Nanoflann access interface. Get the 'dim'th component of the 'index'th
	 * data point.
	 */
	inline double kdtree_get_pt(const size_t index, int dim) const {

		if (dim == 0)
			return _slices[index]->getComponent()->getCenter().x;
		else if (dim == 1)
			return _slices[index]->getComponent()->getCenter().y;
		else return 0;
	}

	/**
	 * Nanoflann access interface. Computes a bounding box for the data or
	 * returns false.
	 */
	template <class BBox>
	bool kdtree_get_bbox(BBox&) const { return false; }

private:

	// a reference to the data to sort into the kd-tree
	const slices_type& _slices;
};

/**
 * A collection of slices.
 */
class Slices : public pipeline::Data {

	typedef std::vector<boost::shared_ptr<Slice> > slices_type;

	// nanoflann kd-tree type
	typedef nanoflann::KDTreeSingleIndexAdaptor<
			nanoflann::L2_Simple_Adaptor<double, SliceVectorAdaptor>,
			SliceVectorAdaptor,
			2>
			SliceKdTree;

public:

	typedef slices_type::iterator       iterator;

	typedef slices_type::const_iterator const_iterator;

	/**
	 * Create a new set of slices.
	 */
	Slices();

	/**
	 * Copy constructor.
	 */
	Slices(const Slices& other);

	~Slices();

	/**
	 * Assignment operator.
	 */
	Slices& operator=(const Slices& other);

	/**
	 * Remove all slices.
	 */
	void clear();

	/**
	 * Add a single slice to this set of slices.
	 */
	void add(boost::shared_ptr<Slice> slice);

	/**
	 * Add a set of slices to this set of slices.
	 */
	void addAll(const Slices& slices);

	/**
	 * Remove the given slice.
	 */
	void remove(boost::shared_ptr<Slice> slice);

	const const_iterator begin() const { return _slices.begin(); }

	iterator begin() { return _slices.begin(); }

	const const_iterator end() const { return _slices.end(); }

	iterator end() { return _slices.end(); }

	unsigned int size() const { return _slices.size(); }

	boost::shared_ptr<Slice> operator[](unsigned int i) { return _slices[i]; }

	/**
	 * Find all slices within distance to the given center.
	 */
	std::vector<boost::shared_ptr<Slice> > find(const util::point<double>& center, double distance);

	/**
	 * Move all slices in 2D.
	 */
	void translate(const util::point<int>& offset);

private:

	// the slices
	slices_type _slices;

	// nanoflann vector adaptor
	SliceVectorAdaptor* _adaptor;

	// a kd-tree, which is created on-demand
	SliceKdTree* _kdTree;

	// indicate that the tree has to be (re)build
	bool _kdTreeDirty;
};

#endif // CELLTRACKER_CELLS_H__

