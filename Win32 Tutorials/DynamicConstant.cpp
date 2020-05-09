#include "DynamicConstant.h"

#define DCB_RESOLVE_BASE(eltype) \
size_t LayoutElement::Resolve ## eltype() const noxnd \
{ \
	assert( false && "Cannot resolve to" #eltype ); return 0u; \
}

#define DCB_LEAF_ELEMENT_IMPL(eltype,systype,hlslSize) \
size_t eltype::Resolve ## eltype() const noxnd \
{ \
	return GetOffsetBegin(); \
} \
size_t eltype::GetOffsetEnd() const noexcept \
{ \
	return GetOffsetBegin() + ComputeSize(); \
} \
size_t eltype::Finalize( size_t offset_in ) \
{ \
	offset = offset_in; \
	return offset_in + ComputeSize(); \
} \
size_t eltype::ComputeSize() const noxnd \
{ \
	return (hlslSize); \
}
#define DCB_LEAF_ELEMENT(eltype,systype) DCB_LEAF_ELEMENT_IMPL(eltype,systype,sizeof(systype))

#define DCB_REF_CONVERSION(reftype,eltype,...) \
reftype::operator __VA_ARGS__ eltype::SystemType&() noxnd \
{ \
	return *reinterpret_cast<eltype::SystemType*>(pBytes + offset + pLayout->Resolve ## eltype()); \
}
#define DCB_REF_ASSIGN(reftype,eltype) \
eltype::SystemType& reftype::operator=( const eltype::SystemType& rhs ) noxnd \
{ \
	return static_cast<eltype::SystemType&>(*this) = rhs; \
}
#define DCB_REF_NONCONST(reftype,eltype) DCB_REF_CONVERSION(reftype,eltype) DCB_REF_ASSIGN(reftype,eltype)
#define DCB_REF_CONST(reftype,eltype) DCB_REF_CONVERSION(reftype,eltype,const)

#define DCB_PTR_CONVERSION(reftype,eltype,...) \
reftype::Ptr::operator __VA_ARGS__ eltype::SystemType*() noxnd \
{ \
	return &static_cast<__VA_ARGS__ eltype::SystemType&>(ref); \
}

namespace Dcb {
	LayoutElement& LayoutElement::operator[](const std::string&) {
		assert(false && "Cannot access member on non struct");
		return *this;
	}
	const LayoutElement& LayoutElement::operator[](const std::string&) const {
		assert(false && "Cannot access member on non struct");
		return *this;
	}
	LayoutElement& LayoutElement::T() {
		assert(false);
		return *this;
	}
	const LayoutElement& LayoutElement::T() const {
		assert(false);
		return *this;
	}
	size_t LayoutElement::GetOffsetBegin() const noexcept
	{
		return offset;
	}
	size_t LayoutElement::GetSizeInBytes() const noexcept {
		return GetOffsetEnd() - GetOffsetBegin();
	}
	size_t LayoutElement::GetNextBoundaryOffset(size_t offset) {
		return offset + (16u - offset % 16u) % 16u;
	}

	DCB_RESOLVE_BASE(Matrix)
		DCB_RESOLVE_BASE(Float4)
		DCB_RESOLVE_BASE(Float3)
		DCB_RESOLVE_BASE(Float2)
		DCB_RESOLVE_BASE(Float)
		DCB_RESOLVE_BASE(Bool)

		DCB_LEAF_ELEMENT(Matrix, dx::XMFLOAT4X4)
		DCB_LEAF_ELEMENT(Float4, dx::XMFLOAT4)
		DCB_LEAF_ELEMENT(Float3, dx::XMFLOAT3)
		DCB_LEAF_ELEMENT(Float2, dx::XMFLOAT2)
		DCB_LEAF_ELEMENT(Float, float)
		DCB_LEAF_ELEMENT_IMPL(Bool, bool, 4u)

	LayoutElement& Struct::operator[](const std::string& key) {
		return *map.at(key);
	}
	const LayoutElement& Struct::operator[](const std::string& key) const {
		return *map.at(key);
	}
	size_t Struct::GetOffsetEnd() const noexcept {
		return LayoutElement::GetNextBoundaryOffset(elements.back()->GetOffsetEnd());
	}
	void Struct::Add(const std::string& name, std::unique_ptr<LayoutElement> pElement) noxnd {
		elements.push_back(std::move(pElement));
		if (!map.emplace(name, elements.back().get()).second) {
			assert(false);
		}
	}
	size_t Struct::Finalize(size_t offset_in) {
		assert(elements.size() != 0u);
		offset = offset_in;
		auto offsetNext = offset;
		for (auto& el : elements) {
			offsetNext = (*el).Finalize(offsetNext);
		}
		return GetOffsetEnd();
	}
	size_t Struct::ComputeSize() const noxnd {
		size_t offsetNext = 0u;
		for (auto& el : elements) {
			const auto elSize = el->ComputeSize();
			offsetNext += CalculatePaddingBeforeElement(offsetNext, elSize) + elSize;
		}

		return GetNextBoundaryOffset(offsetNext);
	}
	size_t Struct::CalculatePaddingBeforeElement(size_t offset, size_t size) noexcept {
		if (offset / 16u != (offset + size - 1) / 16u) {
			return GetNextBoundaryOffset(offset) - offset;
		}
		return offset;
	}
}