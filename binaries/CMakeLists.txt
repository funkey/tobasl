define_module(multi2cut BINARY SOURCES multi2cut.cpp LINKS slices features io imageprocessing)
define_module(test_imagelevelparser BINARY SOURCES test_imagelevelparser.cpp LINKS imageprocessing imageprocessing_gui gui)
