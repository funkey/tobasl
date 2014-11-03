#include <fstream>
#include <vigra/impex.hxx>
#include <util/exceptions.h>
#include "SlicesWriter.h"

SlicesWriter::SlicesWriter(const std::string& directory) :
	_directory(directory) {

	registerInput(_slices, "slices");
}

void
SlicesWriter::write() {

	updateInputs();

	// store slices and their offsets

	std::string sliceOffsetsFilename = _directory + "/offsets.txt";
	std::ofstream sliceOffsets(sliceOffsetsFilename.c_str());

	if (!sliceOffsets)
		UTIL_THROW_EXCEPTION(
				IOError,
				"can not open " << sliceOffsetsFilename << " for writing");

	sliceOffsets << "slice_id\tmin_x\tmin_y\t# (note that min_x and min_y are given for the slice, not for the slice image, which has a padding of one pixel)" << std::endl;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		std::stringstream sliceNumber;
		sliceNumber << std::setw(8) << std::setfill('0') << slice->getId();

		util::rect<unsigned int> boundingBox = slice->getComponent()->getBoundingBox();

		std::string imageFilename = "output_images/slices/slice_" + sliceNumber.str() + ".png";

		const ConnectedComponent::bitmap_type& bitmap = slice->getComponent()->getBitmap();

		storeSliceBitmap(bitmap, imageFilename);

		// save the offset of the slice
		sliceOffsets
				<< sliceNumber.str()
				<< "\t" << boundingBox.minX
				<< "\t" << boundingBox.minY
				<< std::endl;
	}
}

void
SlicesWriter::storeSliceBitmap(const ConnectedComponent::bitmap_type& bitmap, const std::string& filename) {

	vigra::Shape2 size = bitmap.shape();

	// add a margin of one pixel
	size[0] += 2;
	size[1] += 2;

	ConnectedComponent::bitmap_type paddedBitmap(size);
	paddedBitmap = 0;

	// copy the original bitmap
	vigra::copyMultiArray(
			bitmap,
			paddedBitmap.subarray(vigra::Shape2(1, 1), vigra::Shape2(-1, -1)));

	// store the padded bitmap
	vigra::exportImage(
			vigra::srcImageRange(paddedBitmap),
			vigra::ImageExportInfo(filename.c_str()));
}
