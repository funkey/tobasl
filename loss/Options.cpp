#include "Options.h"

util::ProgramOption optionLossMaxCenterDistance(
		util::_module           = "loss",
		util::_long_name        = "maxCenterDistance",
		util::_description_text = "The maximal center distance between candidates and ground truth to consider for computing the slice distance loss.",
		util::_default_value    = 1000);
