#include "Mesh.h"
#include "imgui/imgui.h"
#include "Surface.h"
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include "RedSkyXM.h"
#include "DynamicConstant.h"
#include "ConstantBufferEx.h"
#include "LayoutCodex.h"
#include "Stencil.h"

namespace dx = DirectX;

Mesh::Mesh(Graphics& gfx, const Material& mat, const aiMesh& mesh) noxnd
	: Drawable(gfx,mat,mesh) {}

void Mesh::Submit(FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransform) const noxnd {
	dx::XMStoreFloat4x4(&transform, accumulatedTransform);
	Drawable::Submit(frame);
}




