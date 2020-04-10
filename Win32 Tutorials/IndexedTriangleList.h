#pragma once
#include <vector>
#include <DirectXMath.h>

//Abstract class used for creating shapes 
template<class T>
class IndexedTriangleList {
public:
	IndexedTriangleList() = default;
	IndexedTriangleList(std::vector<T> verts_in, std::vector<unsigned short> indices_in) : vertices(std::move(verts_in)), indices(std::move(indices_in)) {
		//Checks that that there are at least 2 vertices's passed through (minimum 3)
		assert(vertices.size() > 2);

		//Checks that the number of indices is divisible by three 
		assert(indices.size() % 3 == 0);
	}

	//Stores the transform of the object
	void Transform(DirectX::FXMMATRIX matrix) {
		for (auto& v : vertices) {
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
			DirectX::XMStoreFloat3(&v.pos, DirectX::XMVector3Transform(pos, matrix));
		}
	}

public:
	std::vector<T> vertices;
	std::vector<unsigned short> indices;
};
