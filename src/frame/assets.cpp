#include "frame/assets.h"

#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

namespace Mirror {

TextureData::TextureData(const std::string_view png_path)
	: pixels(stbi_load(png_path.data(), &width, &height, &channels, STBI_rgb_alpha)) {
	release_assert(pixels.get() != nullptr);
	release_assert(width > 0);
	release_assert(height > 0);
	release_assert(channels > 0);
}

// TODO: fix index buffer
ModelData::ModelData(const std::string_view obj_path, Vec3<bool> flip) {
	debug_assert(!obj_path.empty());

	tinyobj::ObjReaderConfig reader_config{};
	reader_config.vertex_color = false;
	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(std::string{obj_path}, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << reader.Error() << '\n';
		}
		if (!reader.Warning().empty()) {
			std::cerr << reader.Warning() << '\n';
		}
		release_assert(reader.Valid());
	}
	if (!reader.Error().empty()) {
		std::cerr << reader.Error() << '\n';
	}
	debug_assert(reader.Error().empty());
	if (!reader.Warning().empty()) {
		std::cerr << reader.Warning() << '\n';
	}
	debug_assert(reader.Warning().empty());
	debug_assert(reader.Valid());

	const tinyobj::attrib_t& attrib = reader.GetAttrib();
	release_assert(!attrib.vertices.empty());
	release_assert(!attrib.normals.empty());
	release_assert(!attrib.texcoords.empty());

	for (const auto& shape : reader.GetShapes()) {
		usize index_offset = 0;
		for (u32 face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
			usize num_vertices = shape.mesh.num_face_vertices[face];

			for (u32 vertex = 0; vertex < num_vertices; vertex++) {
				tinyobj::index_t index = shape.mesh.indices[index_offset + vertex];

				indices.emplace_back(static_cast<u32>(indices.size()));
				vertices.emplace_back(
					Vec3f{
						attrib.vertices[3 * static_cast<usize>(index.vertex_index) + 0] * (flip.x ? -1 : 1),
						attrib.vertices[3 * static_cast<usize>(index.vertex_index) + 1] * (flip.y ? -1 : 1),
						attrib.vertices[3 * static_cast<usize>(index.vertex_index) + 2] * (flip.z ? -1 : 1),
					},
					Vec3f{
						attrib.normals[3 * static_cast<usize>(index.normal_index) + 0],
						attrib.normals[3 * static_cast<usize>(index.normal_index) + 1],
						attrib.normals[3 * static_cast<usize>(index.normal_index) + 2],
					},
					Vec2f{
						attrib.texcoords[2 * static_cast<usize>(index.texcoord_index) + 0],
						attrib.texcoords[2 * static_cast<usize>(index.texcoord_index) + 1],
					});
			}

			index_offset += num_vertices;
		}
	}
	release_assert(indices.size() <= UINT32_MAX);

	debug_assert(!indices.empty());
	debug_assert(!vertices.empty());
}

} // namespace Mirror
