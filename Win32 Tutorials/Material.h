#pragma once
#include "BindableCommon.h"
#include <vector>
#include <filesystem>
#include "Technique.h"
#include "Graphics.h"

struct aiMaterial;
struct aiMesh;

class Material {
public:
	Material(Graphics& gfx, const aiMaterial& materail, const std::filesystem::path& path) noxnd;
	rsexp::VertexBuffer ExtractVertices(const aiMesh& mesh) const noexcept;
	std::vector<unsigned short> ExtractIndices(const aiMesh& mesh) const noexcept;
	std::shared_ptr<Bind::VertexBuffer> MakeVertexBindable(Graphics& gfx, const aiMesh& mesh) const noxnd;
	std::shared_ptr<Bind::IndexBuffer> MakeIndexBindable(Graphics& gfx, const aiMesh& mesh) const noxnd;
	std::vector<Technique> GetTechniques() const noexcept;
private:
	std::string MakeMeshTag(const aiMesh& mesh) const noexcept;
private:
	rsexp::VertexLayout vtxLayout;
	std::vector<Technique> techniques;
	std::string modelPath;
	std::string name;
};
