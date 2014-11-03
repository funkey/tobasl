#ifndef MULTI2CUT_IO_SLICES_WRITER_H__
#define MULTI2CUT_IO_SLICES_WRITER_H__

#include <string>
#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>

class SlicesWriter : public pipeline::SimpleProcessNode<> {

public:

	/**
	 * Create a new slice writer that stores slices in the given directory.
	 */
	SlicesWriter(const std::string& directory);

	void write();

private:

	// no outputs
	void updateOutputs() {}

	void storeSliceBitmap(const ConnectedComponent::bitmap_type& bitmap, const std::string& filename);

	pipeline::Input<Slices> _slices;

	std::string _directory;
};

#endif // MULTI2CUT_IO_SLICES_WRITER_H__

