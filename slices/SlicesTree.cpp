#include "SlicesTree.h"
#include <util/Logger.h>

logger::LogChannel slicestreelog("slicestreelog", "[SlicesTree] ");

SlicesTree::Node::Node(boost::shared_ptr<Slice> slice) :
	_slice(slice) {}

void
SlicesTree::Node::setParent(boost::shared_ptr<SlicesTree::Node> parent) {

	_parent = parent;
}

boost::shared_ptr<SlicesTree::Node>
SlicesTree::Node::getParent() {

	boost::shared_ptr<SlicesTree::Node> parent = _parent.lock();

	return parent;
}

void
SlicesTree::Node::addChild(boost::shared_ptr<SlicesTree::Node> sliceNode) {

	_children.push_back(sliceNode);
}

std::vector<boost::shared_ptr<SlicesTree::Node> >
SlicesTree::Node::getChildren() {

	return _children;
}

boost::shared_ptr<Slice>
SlicesTree::Node::getSlice() {

	return _slice;
}

void
SlicesTree::addChild(boost::shared_ptr<Slice> slice) {

	boost::shared_ptr<Node> newChild = boost::make_shared<Node>(slice);

	if (!_current) {

		LOG_DEBUG(slicestreelog) << "adding new root node for slice " << slice->getId() << std::endl;
		_roots.push_back(newChild);

	} else {

		LOG_DEBUG(slicestreelog) << "adding slice " << slice->getId()  << " as child of " << _current->getSlice()->getId() << std::endl;
		_current->addChild(newChild);
		newChild->setParent(_current);
	}

	_current = newChild;

	add(slice);
}

void
SlicesTree::leaveChild() {

	LOG_DEBUG(slicestreelog) << "leaving last added child" << std::endl;

	if (_current) {

		 LOG_DEBUG(slicestreelog) << "current node was " << _current->getSlice()->getId() << std::endl;

		_current = _current->getParent();

		if (_current) {
			 LOG_DEBUG(slicestreelog) << "current node is now " << _current->getSlice()->getId() << std::endl;
		} else {
			 LOG_DEBUG(slicestreelog) << "there is no current node anymore" <<  std::endl;
		}

	} else {

		LOG_DEBUG(slicestreelog) << "there was no last added child -- something might be wrong" << std::endl;
	}
}
