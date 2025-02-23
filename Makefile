run:
	cmake -Bbuild
	cmake --build build
	build/app/Debug/app.exe
test:
	cmake -Bbuild
	cmake --build build
	build/test/Debug/test.exe
release:
	cmake -Bbuild
	cmake --build build --config Release
	build/app/Release/app.exe
shaders:
	C:/VulkanSDK/1.3.283.0/Bin/glslc.exe -o app/src/shaders/sprite.vert.spv app/src/shaders/sprite.vert
	C:/VulkanSDK/1.3.283.0/Bin/glslc.exe -o app/src/shaders/sprite.frag.spv app/src/shaders/sprite.frag
