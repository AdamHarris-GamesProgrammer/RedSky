#pragma once
#include "Drawable.h"
#include "FrameCommander.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Bind;

class Material;
class FrameCommander;
struct aiMesh;

//Mesh is a drawable object
class Mesh : public Drawable {
public:
	Mesh(Graphics& gfx, const Material& mat, const aiMesh& mesh) noxnd;

	DirectX::XMMATRIX GetTransformXM() const noexcept override { return DirectX::XMLoadFloat4x4(&transform); } //This is called in the Drawable::Draw method
	void Submit(FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
private:
	mutable DirectX::XMFLOAT4X4 transform;
};



