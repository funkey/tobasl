#include "ComponentTreeConverter.h"

static logger::LogChannel componenttreeconverterlog("componenttreeconverterlog", "[ComponentTreeConverter] ");

ComponentTreeConverter::ComponentTreeConverter(unsigned int section) :
	_slices(new SlicesTree()),
	_conflictSets(new ConflictSets()),
	_section(section) {

	registerInput(_componentTree, "component tree");
	registerOutput(_slices, "slices");
	registerOutput(_conflictSets, "conflict sets");
}

unsigned int
ComponentTreeConverter::getNextSliceId() {

	unsigned int id;
	
	{
		boost::mutex::scoped_lock lock(SliceIdMutex);

		id = NextSliceId;

		NextSliceId++;
	}

	return id;
}

void
ComponentTreeConverter::updateOutputs() {

	convert();
}

void
ComponentTreeConverter::convert() {

	LOG_DEBUG(componenttreeconverterlog) << "converting component tree to slices..." << std::endl;

	_slices->clear();

	_conflictSets->clear();

	// skip the fake root
	foreach (boost::shared_ptr<ComponentTree::Node> node, _componentTree->getRoot()->getChildren())
		_componentTree->visit(node, *this);

	LOG_DEBUG(componenttreeconverterlog) << "extracted " << _slices->size() << " slices" << std::endl;
}

void
ComponentTreeConverter::visitNode(boost::shared_ptr<ComponentTree::Node> node) {

	LOG_DEBUG(componenttreeconverterlog) << "visiting a node" << std::endl;

	unsigned int sliceId = getNextSliceId();

	_path.push_back(sliceId);

	boost::shared_ptr<ConnectedComponent> component = node->getComponent();

	_slices->addChild(boost::make_shared<Slice>(sliceId, _section, component));

	LOG_ALL(componenttreeconverterlog) << "extracted a slice at " << component->getCenter() << std::endl;

	// for leafs
	if (node->getChildren().size() == 0)
		addConflictSet();
}

void
ComponentTreeConverter::leaveNode(boost::shared_ptr<ComponentTree::Node>) {

	LOG_DEBUG(componenttreeconverterlog) << "leaving a node" << std::endl;

	_path.pop_back();

	_slices->leaveChild();
}

void
ComponentTreeConverter::addConflictSet() {

	LOG_ALL(componenttreeconverterlog) << "found a leaf node, creating a conflict set for it" << std::endl;

	ConflictSet conflictSet;

	foreach (unsigned int sliceId, _path)
		conflictSet.addSlice(sliceId);

	_conflictSets->add(conflictSet);
}

unsigned int ComponentTreeConverter::NextSliceId = 0;
boost::mutex ComponentTreeConverter::SliceIdMutex;
