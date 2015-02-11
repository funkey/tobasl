#include <boost/filesystem.hpp>
#include <util/Logger.h>
#include "MergeTreeReader.h"
#include "MultiMergeTreeReader.h"

logger::LogChannel multimergetreereaderlog("multimergetreereaderlog", "[MultiMergeTreeReader] ");

MultiMergeTreeReader::MultiMergeTreeReader(std::string mergeTreesDirectory) {


	// list all directories under ./slices for the image stack option
	boost::filesystem::path dir(mergeTreesDirectory);

	if (!boost::filesystem::exists(dir))
		BOOST_THROW_EXCEPTION(IOError() << error_message(dir.string() + " does not exist"));

	if (!boost::filesystem::is_directory(dir))
		BOOST_THROW_EXCEPTION(IOError() << error_message(dir.string() + " is not a directory"));

	// get a sorted list of the directory contents
	std::vector<boost::filesystem::path> sorted;
	std::copy(
			boost::filesystem::directory_iterator(dir),
			boost::filesystem::directory_iterator(),
			back_inserter(sorted));
	std::sort(sorted.begin(), sorted.end());

	LOG_DEBUG(multimergetreereaderlog) << "merge tree directory contains " << sorted.size() << " entries" << std::endl;

	// for every image in the directory
	foreach (boost::filesystem::path file, sorted)
		if (!boost::filesystem::is_directory(file)) {

			pipeline::Process<MergeTreeReader> reader(file.string());

			_slicesCollector->addInput("slices", reader->getOutput("slices"));
			_slicesCollector->addInput("conflict sets", reader->getOutput("conflict sets"));
		}

	registerOutput(_slicesCollector->getOutput("slices"), "slices");
	registerOutput(_slicesCollector->getOutput("conflict sets"), "conflict sets");
}
