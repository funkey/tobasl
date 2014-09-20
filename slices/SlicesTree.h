#ifndef MULTI2CUT_SLICES_SLICES_TREE_H__
#define MULTI2CUT_SLICES_SLICES_TREE_H__

#include "Slices.h"

class SlicesTree : public Slices {

public:

	/**
	 * A node in the slices tree. Stores the slice and pointers to the children 
	 * and parent node(s). Pointers to the slice and children are shared 
	 * pointers, pointers to the parent weak pointers. It is thereby enough to 
	 * keep the root of the tree alive to ensure that every connected component 
	 * in the tree is still available.
	 */
	class Node {

	public:

		/**
		 * Create a new node from a slice.
		 *
		 * @param component The slice that is to be represented by
		 *                  this node.
		 */
		Node(boost::shared_ptr<Slice> slice);

		/**
		 * Set the parent of this node.
		 *
		 * @param A shared pointer to the new parent.
		 */
		void setParent(boost::shared_ptr<Node> parent);

		/**
		 * Get the parent of this node.
		 *
		 * @return The parent of this node.
		 */
		boost::shared_ptr<Node> getParent();

		/**
		 * Add a child to this node.
		 *
		 * @param A shared pointer to the new child.
		 */
		void addChild(boost::shared_ptr<Node> componentNode);

		/**
		 * Get all children of this node.
		 *
		 * @return A vector of shared pointers to the children of this node.
		 */
		std::vector<boost::shared_ptr<Node> > getChildren();

		/**
		 * Get the slice represented by this node.
		 *
		 * @return The slice help by this node.
		 */
		boost::shared_ptr<Slice> getSlice();

	private:

		boost::shared_ptr<Slice> _slice;

		boost::weak_ptr<Node> _parent;

		std::vector<boost::shared_ptr<Node> > _children;
	};

	/**
	 * Add a new slice as a child of the current (i.e., previously added) slice.
	 */
	void addChild(boost::shared_ptr<Slice> slice);

	/**
	 * Go up on level, i.e., make the parent of the current slice the new 
	 * current slice.
	 */
	void leaveChild();

	const std::vector<boost::shared_ptr<Node> >& getRoots() const { return _roots; }

private:

	std::vector<boost::shared_ptr<Node> > _roots;

	boost::shared_ptr<Node> _current;
};

#endif // MULTI2CUT_SLICES_SLICES_TREE_H__

