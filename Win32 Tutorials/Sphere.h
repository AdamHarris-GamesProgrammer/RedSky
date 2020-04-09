#pragma once
#include "IndexedTriangleList.h"
#include "RedSkyMath.h"


class Sphere {
public:
	template<class V>
	static IndexedTriangleList<V> MakeTesseltated(int latDiv, int longDiv) {
		namespace DX = DirectX;
		assert(latDiv >= 3);
		assert(longDiv >= 3);

		constexpr float radius = 1.0f;
		const auto base = DX::XMVectorSet(0.0f, 0.0f, radius, 0.0f);
		const float lattitudeAngle = PI / latDiv;
		const float longitudeAngle = 2.0f * PI / longDiv;

		std::vector<V> vertices;
		for (int iLat = 1; iLat < latDiv; iLat++) {
			const latBase = DX::XMVector3Transform(base, DX::XMMatrixRotationX(lattitudeAngle * iLat);

			for (int iLong = 0; iLong < longDiv; iLong++) {
				vertices.emplace_back();
				auto v = DX::XMVector3Transform(latBase, DX:XMMatrixRotationZ(longitudeAngle * iLong));

				DX::XMStoreFloat3(&vertices.back().pos.v);
			}
		}

		//Cap verts
		const auto iNorthPole = (unsigned short)vertices.size();
		vertices.emplace_back();
		DX::XMStoreFloat3(&vertices.back().pos, base);
		const auto iSouthPole = (unsigned short)vertices.size();
		vertices.emplace_back();
		DX::XMStoreFloat3(&vertices.back().pos, DX::XMVectorNegate(base));

		const auto calcIdx = [latDiv, longDiv](unsigned short iLat, unsigned short iLong) { return iLat * longDiv + iLong; };

		std::vector<unsigned short> indices;
		for (unsigned short iLat = 0; iLat < latDiv - 2; iLat++) {
			for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++) {
				indices.push_back(calcIdx(iLat, iLong));
				indices.push_back(calcIdx(iLat + 1, iLong));
				indices.push_back(calcIdx(iLat, iLong + 1));
				indices.push_back(calcIdx(iLat, iLong + 1));
				indices.push_back(calcIdx(iLat + 1, iLong));
				indices.push_back(calcIdx(iLat + 1, iLong + 1));
			}

			//wrap 
			indices.push_back(calcIdx(iLat, longDiv - 1));
			indices.push_back(calcIdx(iLat + 1, longDiv - 1));
			indices.push_back(calcIdx(iLat, 0));
			indices.push_back(calcIdx(iLat, 0));
			indices.push_back(calcIdx(iLat + 1, longDiv - 1));
			indices.push_back(calcIdx(iLat + 1, 0));
		}

		//cap
		for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++) {
			//north
			indices.push_back(iNorthPole);
			indices.push_back(calcIdx(0, iLong));
			indices.push_back(calcIdx(0, iLong + 1));

			//south
			indices.push_back(calcIdx(latDiv - 2, iLong + 1));
			indices.push_back(calcIdx(latDiv - 2, iLong));
			indices.push_back(iSouthPole);
		}

		// wrap triangles
		// north
		indices.push_back(iNorthPole);
		indices.push_back(calcIdx(0, longDiv - 1));
		indices.push_back(calcIdx(0, 0));
		// south
		indices.push_back(calcIdx(latDiv - 2, 0));
		indices.push_back(calcIdx(latDiv - 2, longDiv - 1));
		indices.push_back(iSouthPole);

		return { std::move(vertices), std::move(indices) };
	}

	template<class V>
	static IndexedTriangleList<V> Make() {
		return MakeTesseltated(12, 24);
	}
};