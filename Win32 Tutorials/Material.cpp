#include "Material.h"

Material::Material(Graphics& gfx, const aiMaterial& material, const std::filesystem::path& path) noxnd
	: modelPath(path.string())
{
	using namespace Bind;
	const auto rootPath = path.parent_path().string() + "\\"; 
	{
		aiString tempName;
		material.Get(AI_MATKEY_NAME, tempName);
		name = tempName.C_Str();
	}

	//Phong
	{
		Technique phong{ "Phong" };
		Step step(0);
		std::string shaderCode = "Phong";
		aiString texFileName;

		//Common
		vtxLayout.Append(rsexp::VertexLayout::Position3D);
		vtxLayout.Append(rsexp::VertexLayout::Normal);
		Dcb::RawLayout pscLayout;
		bool hasTexture = false;
		bool hasGlossAlpha = false;

		//Diffuse
		{
			bool hasAlpha = false;
			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS) {
				hasTexture = true;
				shaderCode += "Dif";
				vtxLayout.Append(rsexp::VertexLayout::Texture2D);
				auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str());
				if (tex->HasAlpha()) {
					hasAlpha = true;
					shaderCode += "Msk";
				}
				step.AddBindable(std::move(tex));
			}
			else
			{
				pscLayout.Add<Dcb::Float3>("materialColor");
			}
			step.AddBindable(Rasterizer::Resolve(gfx, hasAlpha));
		}

		//Specular
		{
			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS) {
				hasTexture = true;
				shaderCode += "Spc";
				vtxLayout.Append(rsexp::VertexLayout::Texture2D);
				auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 1);
				hasGlossAlpha = tex->HasAlpha();
				step.AddBindable(std::move(tex));
				pscLayout.Add<Dcb::Bool>("useGlossAlpha");
			}
			pscLayout.Add<Dcb::Float3>("specularColor");
			pscLayout.Add<Dcb::Float>("specularWeight");
			pscLayout.Add<Dcb::Float>("specularGloss");
		}

		//Normal
		{
			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS) {
				hasTexture = true;
				shaderCode += "Nrm";
				vtxLayout.Append(rsexp::VertexLayout::Texture2D);
				vtxLayout.Append(rsexp::VertexLayout::Tangent);
				vtxLayout.Append(rsexp::VertexLayout::Bitangent);
				step.AddBindable(Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 2));
				pscLayout.Add<Dcb::Bool>("useNormalMap");
				pscLayout.Add<Dcb::Float>("normalMapWeight");
			}
		}

		//common post processing
		{
			step.AddBindable(std::make_shared<TransformCbuf>(gfx, 0u));
			step.AddBindable(Blender::Resolve(gfx, false));
			auto pvs = VertexShader::Resolve(gfx, shaderCode + "_VS.cso");
			auto pvsbc = pvs->GetByteCode();
			step.AddBindable(std::move(pvs));
			step.AddBindable(PixelShader::Resolve(gfx, shaderCode + "_PS.cso"));
			step.AddBindable(InputLayout::Resolve(gfx, vtxLayout, pvsbc));
			if (hasTexture) {
				step.AddBindable(Bind::Sampler::Resolve(gfx));
			}

			//PS Material Parameters
			Dcb::Buffer buf{ std::move(pscLayout) };
			if (auto r = buf["materialColor"]; r.Exists()) {
				aiColor3D color = { 0.45f,0.45f,0.85f };
				material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
				r = reinterpret_cast<DirectX::XMFLOAT3&>(color);
			}
			buf["useGlossAlpha"].SetIfExists(hasGlossAlpha);
			if (auto r = buf["specularColor"]; r.Exists()) {
				aiColor3D color = { 0.45f,0.45f,0.85f };
				material.Get(AI_MATKEY_COLOR_SPECULAR, color);
				r = reinterpret_cast<DirectX::XMFLOAT3&>(color);
			}
			buf["specularWeight"].SetIfExists(1.0f);
			if (auto r = buf["specularGloss"]; r.Exists()) {
				float gloss = 8.0f;
				material.Get(AI_MATKEY_SHININESS, gloss);
				r = gloss;
			}
			buf["useNormalMap"].SetIfExists(true);
			buf["normalMapWeight"].SetIfExists(1.0f);
			step.AddBindable(std::make_unique<Bind::CachingPixelConstantBufferEX>(gfx, std::move(buf), 1u));
		}
		phong.AddStep(std::move(step));
		techniques.push_back(std::move(phong));
	}

	//outline techniques
	{
		Technique outline("Outline");
		{
			Step mask(1);

			auto pvs = VertexShader::Resolve(gfx, "Solid_VS.cso");
			auto pvsbc = pvs->GetByteCode();
			mask.AddBindable(std::move(pvs));

			mask.AddBindable(InputLayout::Resolve(gfx, vtxLayout, pvsbc));

			mask.AddBindable(std::make_shared<TransformCbuf>(gfx));

			outline.AddStep(std::move(mask));
		}
		{
			Step draw(2);

			auto pvs = VertexShader::Resolve(gfx, "Solid_VS.cso");
			auto pvsbc = pvs->GetByteCode();
			draw.AddBindable(std::move(pvs));

			draw.AddBindable(PixelShader::Resolve(gfx, "Solid_PS.cso"));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Float3>("materialColor");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["materialColor"] = DirectX::XMFLOAT3{ 1.0f, 0.4f,0.4f};
			draw.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEX>(gfx, buf, 1u));

			draw.AddBindable(InputLayout::Resolve(gfx, vtxLayout, pvsbc));

			class TransfromCbufScaling : public TransformCbuf {
			public:
				TransfromCbufScaling(Graphics& gfx, float scale = 1.04f)
					: TransformCbuf(gfx), buf(MakeLayout()) 
				{
					buf["scale"] = scale;
				}
				void Accept(TechniqueProbe& probe) {
					probe.VisitBuffer(buf);
				}
				void Bind(Graphics& gfx) noexcept override {
					const float scale = buf["scale"];
					const auto scaleMatrix = DirectX::XMMatrixScaling(scale, scale, scale);
					auto xf = GetTransforms(gfx);
					xf.modelView = xf.modelView * scaleMatrix;
					xf.modelViewProj = xf.modelViewProj * scaleMatrix;
					UpdateBindImpl(gfx, xf);
				}
				std::unique_ptr<CloningBindable> Clone() const noexcept override {
					return std::make_unique<TransfromCbufScaling>(*this);
				}
			private:
				static Dcb::RawLayout MakeLayout() {
					Dcb::RawLayout layout;
					layout.Add<Dcb::Float>("scale");
					return layout;
				}
			private:
				Dcb::Buffer buf;
			};
			draw.AddBindable(std::make_shared<TransfromCbufScaling>(gfx));

			outline.AddStep(std::move(draw));
		}
		techniques.push_back(std::move(outline));
	}
}

rsexp::VertexBuffer Material::ExtractVertices(const aiMesh& mesh) const noexcept
{
	return { vtxLayout, mesh };
}

std::vector<unsigned short> Material::ExtractIndices(const aiMesh& mesh) const noexcept
{
	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
	return indices;
}

std::shared_ptr<Bind::VertexBuffer> Material::MakeVertexBindable(Graphics& gfx, const aiMesh& mesh) const noxnd
{
	return Bind::VertexBuffer::Resolve(gfx, MakeMeshTag(mesh), ExtractVertices(mesh));
}

std::shared_ptr<Bind::IndexBuffer> Material::MakeIndexBindable(Graphics& gfx, const aiMesh& mesh) const noxnd
{
	return Bind::IndexBuffer::Resolve(gfx, MakeMeshTag(mesh), ExtractIndices(mesh));
}

std::vector<Technique> Material::GetTechniques() const noexcept
{
	return techniques;
}

std::string Material::MakeMeshTag(const aiMesh& mesh) const noexcept
{
	return modelPath + "%" + mesh.mName.C_Str();
}
