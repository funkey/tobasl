#ifndef SOPNET_SLICE_EXTRACTOR_H__
#define SOPNET_SLICE_EXTRACTOR_H__

#include <deque>

#include <boost/shared_ptr.hpp>

#include <pipeline/all.h>
#include <inference/LinearConstraints.h>
#include <imageprocessing/ComponentTree.h>
#include <imageprocessing/ComponentTreeExtractorParameters.h>
#include "Slices.h"

// forward declaration
class ComponentTreeDownSampler;
class ComponentTreePruner;
class ComponentTreeConverter;
template <typename Precision> class ComponentTreeExtractor;
class ComponentTreeExtractorParameters;

template <typename Precision>
class SliceExtractor : public pipeline::ProcessNode {

public:

	/**
	 * Create a new slice extractor for the given section.
	 *
	 * @param section
	 *              The section number that the extracted slices will have.
	 *
	 * @param downsample
	 *              Do not extract slices that are single children of their 
	 *              parents in the component tree.
	 */
	SliceExtractor(unsigned int section, bool downsample);

private:

	void extractSlices();

	boost::shared_ptr<ComponentTreeExtractor<Precision> > _componentTreeExtractor;
	boost::shared_ptr<ComponentTreeExtractorParameters>   _defaultComponentTreeExtractorParameters;
	boost::shared_ptr<ComponentTreeDownSampler>           _downSampler;
	boost::shared_ptr<ComponentTreePruner>                _pruner;
	boost::shared_ptr<ComponentTreeConverter>             _converter;
};

#endif // SOPNET_SLICE_EXTRACTOR_H__

