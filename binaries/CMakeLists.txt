define_module(merge_tree BINARY SOURCES merge_tree.cpp LINKS mergetree util)
define_module(multi2cut BINARY SOURCES multi2cut.cpp LINKS slices features io imageprocessing)
define_module(test_imagelevelparser BINARY SOURCES test_imagelevelparser.cpp LINKS imageprocessing imageprocessing_gui gui)
