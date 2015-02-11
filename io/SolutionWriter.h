#ifndef MULTI2CUT_IO_SOLUTION_WRITER_H__
#define MULTI2CUT_IO_SOLUTION_WRITER_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>

class SolutionWriter : public pipeline::SimpleProcessNode<> {

public:

	SolutionWriter(unsigned int width, unsigned int height, const std::string& filename = "solution.png");

	void write();

private:

	void updateOutputs() {}

	pipeline::Input<Slices> _solution;

	unsigned int _width;
	unsigned int _height;

	std::string _filename;
};

#endif // MULTI2CUT_IO_SOLUTION_WRITER_H__

