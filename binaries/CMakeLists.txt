define_module(merge_tree BINARY SOURCES merge_tree.cpp LINKS mergetree util)
define_module(multi2cut BINARY SOURCES multi2cut.cpp LINKS slices features io imageprocessing)
define_module(combine_images BINARY SOURCES combine_images.cpp LINKS vigra-git)
