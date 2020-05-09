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
#pragma region Layout Element Class
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
#pragma endregion Layout Element Class

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

#pragma region Struct Class
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
#pragma endregion Struct Class

#pragma region Array Class
	size_t Array::GetOffsetEnd() const noexcept  {
		assert(pElement);
		return GetOffsetBegin() + LayoutElement::GetNextBoundaryOffset(pElement->GetSizeInBytes()) * size;
	}
	void Array::Set(std::unique_ptr<LayoutElement> pElement, size_t size_in) noxnd {
		pElement = std::move(pElement);
		size = size_in;
	}
	LayoutElement& Array::T()  {
		return *pElement;
	}
	const LayoutElement& Array::T() const  {
		return *pElement;
	}
	size_t Array::Finalize(size_t offset_in) {
		assert(size != 0u && pElement);
		offset = offset_in;
		pElement->Finalize(offset_in);
		return GetOffsetEnd();
	}
	size_t Array::ComputeSize() const noxnd  {
		return LayoutElement::GetNextBoundaryOffset(pElement->ComputeSize()) * size;
	}
#pragma endregion Array Class

#pragma region Layout Class
	LayoutElement& Layout::operator[](const std::string& key) {
		assert(!finalized && "Cannot modify finalized layout");
		return (*pLayout)[key];
	}
	size_t Layout::GetSizeInBytes() const noexcept {
		return pLayout->GetSizeInBytes();
	}
	std::shared_ptr<LayoutElement> Layout::Finalize() {
		pLayout->Finalize(0);
		finalized = true;
		return pLayout;
	}
#pragma endregion Layout Class

#pragma region Constant Element Reference Class
	DCB_REF_CONST(ConstElementRef, Matrix, const)
	DCB_REF_CONST(ConstElementRef, Float4, const)
	DCB_REF_CONST(ConstElementRef, Float3, const)
	DCB_REF_CONST(ConstElementRef, Float2, const)
	DCB_REF_CONST(ConstElementRef, Float, const)
	DCB_REF_CONST(ConstElementRef, Bool, const)

	ConstElementRef ConstElementRef::operator[](const std::string& key) noxnd {
		return { &(*pLayout)[key],pBytes,offset };
	}
	ConstElementRef ConstElementRef::operator[](int index) noxnd {
		const auto& t = pLayout->T();
		const auto elementSize = LayoutElement::GetNextBoundaryOffset(t.GetSizeInBytes());
		return { &t, pBytes, offset + elementSize * index };
	}
	ConstElementRef::Ptr ConstElementRef::operator&() noxnd {
		return { *this };
	}

#pragma endregion Constant Element Reference Class

#pragma region Element Reference Class
	DCB_PTR_CONVERSION(ElementRef, Matrix)
	DCB_PTR_CONVERSION(ElementRef, Float4)
	DCB_PTR_CONVERSION(ElementRef, Float3)
	DCB_PTR_CONVERSION(ElementRef, Float2)
	DCB_PTR_CONVERSION(ElementRef, Float)
	DCB_PTR_CONVERSION(ElementRef, Bool)

	ElementRef::operator ConstElementRef() const noexcept {
		return { pLayout, pBytes, offset };
	}

	ElementRef ElementRef::operator[](const std::string& key) noxnd {
		return { &(*pLayout)[key], pBytes,offset };
	}
	ElementRef ElementRef::operator[](size_t index) noxnd {
		const auto& t = pLayout->T();
		const auto elementSize = LayoutElement::GetNextBoundaryOffset(t.GetSizeInBytes());
		return { &t, pBytes, offset + elementSize * index };
	}
	ElementRef::Ptr ElementRef::operator&() noxnd {
		return { *this };
	}

	DCB_REF_NONCONST(ElementRef, Matrix)
	DCB_REF_NONCONST(ElementRef, Float4)
	DCB_REF_NONCONST(ElementRef, Float3)
	DCB_REF_NONCONST(ElementRef, Float2)
	DCB_REF_NONCONST(ElementRef, Float)
	DCB_REF_NONCONST(ElementRef, Bool)
#pragma endregion Element Reference Class

#pragma region Buffer Class
	ElementRef Buffer::operator[](const std::string& key) noxnd {
		return { &(*pLayout)[key],bytes.data(),0u };
	}
	ConstElementRef Buffer::operator[](const std::string& key) const noxnd {
		return const_cast<Buffer&>(*this)[key];
	}
	const char* Buffer::GetData() const noexcept {
		return bytes.data();
	}
	size_t Buffer::GetSizeInBytes() const noexcept {
		return bytes.size();
	}
	const LayoutElement& Buffer::GetLayout() const noexcept {
		return *pLayout;
	}
	std::shared_ptr<LayoutElement> Buffer::CloneLayout() const {
		return pLayout;
	}
#pragma endregion BufferClass
}