SOURCES := $(wildcard shaders/*.vert) $(wildcard shaders/*.frag)
SPV_FILES := $(SOURCES:.vert=.vert.spv)
SPV_FILES := $(SPV_FILES:.frag=.frag.spv)

shaders: $(SPV_FILES)

%.vert.spv: %.vert
	$(VULKAN_SDK)/Bin/glslc.exe $< -o $@

%.frag.spv: %.frag
	$(VULKAN_SDK)/Bin/glslc.exe $< -o $@

