#pragma once
#include "ConditionalNoexcept.h"
#include <cassert>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <numeric>

#define RESOLVE_BASE(eltype) \
virtual size_t Resolve ## eltype() const noxnd \
{ \
	assert(false && "Cannot resolve to" #eltype); return 0u; \
#define DCB_RESOLVE_BASE(eltype) \

#define LEAF_ELEMENT_IMPL(eltype, systype, hlslSize) \
class eltype : public LayoutElement \
{ \
public: \
	using SystemType = systype; \
	size_t Resolve ## eltype() const noxnd override final;\
	size_t GetOffsetEnd() const noexcept override final;\
protected: \
	size_t Finalize( size_t offset_in ) override final;\
	size_t ComputeSize() const noxnd override final;\
};
#define DCB_LEAF_ELEMENT(eltype,systype) DCB_LEAF_ELEMENT_IMPL(eltype,systype,sizeof(systype))

#define DCB_REF_CONVERSION(eltype,...) \
operator __VA_ARGS__ eltype::SystemType&() noxnd;
#define DCB_REF_ASSIGN(eltype) \
eltype::SystemType& operator=( const eltype::SystemType& rhs ) noxnd;
#define DCB_REF_NONCONST(eltype) DCB_REF_CONVERSION(eltype) DCB_REF_ASSIGN(eltype)
#define DCB_REF_CONST(eltype) DCB_REF_CONVERSION(eltype,const)

#define DCB_PTR_CONVERSION(eltype,...) \
operator __VA_ARGS__ eltype::SystemType*() noxnd;

namespace Dcb {
	class Struct;
	class Array;
	class Layout;

	namespace dx = DirectX;
	class LayoutElement {
		friend class Layout;
		friend class Array;
		friend class Struct;
	public:

		virtual ~LayoutElement() {}
		//[] only works for structs, access member by name
		virtual LayoutElement& operator[](const std::string&) {
			assert(false && "Cannot access member on non struct");
			return *this;
		}
		virtual const LayoutElement& operator[](const std::string&) const {
			assert(false && "Cannot access member on non struct");
			return *this;
		}
		//T() only works for arrays
		virtual LayoutElement& T() {
			assert(false);
			return *this;
		}
		virtual const LayoutElement& T() const {
			assert(false);
			return *this;
		}
		//offset based -only works after finalization
		size_t GetOffsetBegin() const noexcept
		{
			return offset;
		}
		virtual size_t GetOffsetEnd() const noexcept = 0;
		//Gets size in bytes derived from offsets
		size_t GetSizeInBytes() const noexcept {
			return GetOffsetEnd() - GetOffsetBegin();
		}

		//Only works for structs
		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd;
		//Only works for arrays
		template<typename T>
		LayoutElement& Set(size_t size) noxnd;

		//Returns the value of the offset up to the next 16byte boundary
		static size_t GetNextBoundaryOffset(size_t offset) {
			return offset + (16u - offset % 16u) % 16u;
		}

		DCB_RESOLVE_BASE(Matrix)
		DCB_RESOLVE_BASE(Float4)
		DCB_RESOLVE_BASE(Float3)
		DCB_RESOLVE_BASE(Float2)
		DCB_RESOLVE_BASE(Float)
		DCB_RESOLVE_BASE(Bool)
	protected:
		//sets all offsets for elements and sub-elements 
		virtual size_t Finalize(size_t offset) = 0;

		virtual size_t ComputeSize() const noxnd = 0;
	protected:
		size_t offset = 0u;
	};

	DCB_LEAF_ELEMENT(Matrix, dx::XMFLOAT4X4)
	DCB_LEAF_ELEMENT(Float4, dx::XMFLOAT4)
	DCB_LEAF_ELEMENT(Float3, dx::XMFLOAT3)
	DCB_LEAF_ELEMENT(Float2, dx::XMFLOAT2)
	DCB_LEAF_ELEMENT(Float, float)
	DCB_LEAF_ELEMENT_IMPL(Bool, bool, 4u)

	class Struct : public LayoutElement {
	public:
		LayoutElement& operator[](const std::string& key) override final {
			return *map.at(key);
		}
		const LayoutElement& operator[](const std::string& key) const override final {
			return *map.at(key);
		}
		size_t GetOffsetEnd() const noexcept override final {
			return LayoutElement::GetNextBoundaryOffset(elements.back()->GetOffsetEnd());
		}
		void Add(const std::string& name, std::unique_ptr<LayoutElement> pElement) noxnd{
			elements.push_back(std::move(pElement));
			if (!map.emplace(name, elements.back().get()).second) {
				assert(false);
			}
		}
	protected:
		size_t Finalize(size_t offset_in) override final {
			assert(elements.size() != 0u);
			offset = offset_in;
			auto offsetNext = offset;
			for (auto& el : elements) {
				offsetNext = (*el).Finalize(offsetNext);
			}
			return GetOffsetEnd();
		}
		size_t ComputeSize() const noxnd override final {
			size_t offsetNext = 0u;
			for (auto& el : elements) {
				const auto elSize = el->ComputeSize();
				offsetNext += CalculatePaddingBeforeElement(offsetNext, elSize) + elSize;
			}

			return GetNextBoundaryOffset(offsetNext);
		}
	private:
		static size_t CalculatePaddingBeforeElement(size_t offset, size_t size) noexcept {
			if (offset / 16u != (offset + size - 1) / 16u) {
				return GetNextBoundaryOffset(offset) - offset;
			}
			return offset;
		}
	private:
		std::unordered_map<std::string,LayoutElement*> map;
		std::vector<std::unique_ptr<LayoutElement>> elements;
	};

	class Array : public LayoutElement {
	public:
		size_t GetOffsetEnd() const noexcept override final {
			assert(pElement);
			return GetOffsetBegin() + LayoutElement::GetNextBoundaryOffset(pElement->GetSizeInBytes()) * size;
		}
		void Set(std::unique_ptr<LayoutElement> pElement, size_t size_in) noxnd {
			pElement = std::move(pElement);
			size = size_in;
		}
		LayoutElement& T() override final {
			return *pElement;
		}
		const LayoutElement& T() const override final {
			return *pElement;
		}
	protected:
		size_t Finalize(size_t offset_in) override final {
			assert(size != 0u && pElement);
			offset = offset_in;
			pElement->Finalize(offset_in);
			return GetOffsetEnd();
		}
		size_t ComputeSize() const noxnd override final {
			return LayoutElement::GetNextBoundaryOffset(pElement->ComputeSize()) * size;
		}
	private:
		size_t size = 0u;
		std::unique_ptr<LayoutElement> pElement;
	};

	class Layout {
	public:
		Layout() : pLayout(std::make_shared<Struct>()) {}

		LayoutElement& operator[](const std::string& key) {
			assert(!finalized && "Cannot modify finalized layout");
			return (*pLayout)[key];
		}
		Layout(std::shared_ptr<LayoutElement> pLayout)
			: pLayout(std::move(pLayout)) {}

		size_t GetSizeInBytes() const noexcept {
			return pLayout->GetSizeInBytes();
		}
		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd {
			assert(!finalized && "Cannot modify finalized layout");
			return pLayout->Add<T>(key);
		}
		std::shared_ptr<LayoutElement> Finalize() {
			pLayout->Finalize(0);
			finalized = true;
			return pLayout;
		}

	private:
		bool finalized = false;
		std::shared_ptr<LayoutElement> pLayout;
	};

	class ConstElementRef {
	public:
		class Ptr {
		public:
			Ptr(ConstElementRef& ref) : ref(ref) {}

			DCB_PTR_CONVERSION(Matrix, const)
			DCB_PTR_CONVERSION(Float4, const)
			DCB_PTR_CONVERSION(Float3, const)
			DCB_PTR_CONVERSION(Float2, const)
			DCB_PTR_CONVERSION(Float, const)
			DCB_PTR_CONVERSION(Bool, const)
		private:
			ConstElementRef& ref;
		};

	public:
		ConstElementRef(const LayoutElement* pLayout, char* pBytes, size_t offset) 
			: offset(offset), pLayout(pLayout), pBytes(pBytes) {}

		ConstElementRef operator[](const std::string& key) noxnd {
			return { &(*pLayout)[key],pBytes,offset };
		}
		ConstElementRef operator[](int index) noxnd {
			const auto& t = pLayout->T();
			const auto elementSize = LayoutElement::GetNextBoundaryOffset(t.GetSizeInBytes());
			return { &t, pBytes, offset + elementSize * index };
		}

		Ptr operator&() noxnd {
			return { *this };
		}

		DCB_REF_CONST(Matrix)
		DCB_REF_CONST(Float4)
		DCB_REF_CONST(Float3)
		DCB_REF_CONST(Float2)
		DCB_REF_CONST(Float)
		DCB_REF_CONST(Bool)
	private:
		size_t offset;
		const class LayoutElement* pLayout;
		char* pBytes;
	};

	class ElementRef {
	public:
		class Ptr {
		public:
			Ptr(ElementRef& ref) : ref(ref) {}

			DCB_PTR_CONVERSION(Matrix)
			DCB_PTR_CONVERSION(Float4)
			DCB_PTR_CONVERSION(Float3)
			DCB_PTR_CONVERSION(Float2)
			DCB_PTR_CONVERSION(Float)
			DCB_PTR_CONVERSION(Bool)
		private:
			ElementRef& ref;
		};

		ElementRef(const LayoutElement* pLayout, char* pBytes, size_t offset)
			: pLayout(pLayout), pBytes(pBytes), offset(offset) {}

		operator ConstElementRef() const noexcept {
			return { pLayout, pBytes, offset };
		}

		ElementRef operator[](const std::string& key) noxnd {
			return { &(*pLayout)[key], pBytes,offset };
		}
		ElementRef operator[](size_t index) noxnd {
			const auto& t = pLayout->T();
			const auto elementSize = LayoutElement::GetNextBoundaryOffset(t.GetSizeInBytes());
			return { &t, pBytes, offset + elementSize * index };
		}
		Ptr operator&() noxnd {
			return { *this };
		}
		
		DCB_REF_NONCONST(Matrix)
		DCB_REF_NONCONST(Float4)
		DCB_REF_NONCONST(Float3)
		DCB_REF_NONCONST(Float2)
		DCB_REF_NONCONST(Float)
		DCB_REF_NONCONST(Bool)

	private:
		const class LayoutElement* pLayout;
		char* pBytes;
		size_t offset;
	};

	class Buffer {
	public:
		Buffer(Layout& lay)
			: pLayout(std::static_pointer_cast<Struct>(lay.Finalize())) 
				,bytes(pLayout->GetOffsetEnd()) {}

		ElementRef operator[](const std::string& key) noxnd {
			return { &(*pLayout)[key],bytes.data(),0u };
		}
		ConstElementRef operator[](const std::string& key) const noxnd {
			return const_cast<Buffer&>(*this)[key];
		}
		const char* GetData() const noexcept {
			return bytes.data();
		}
		size_t GetSizeInBytes() const noexcept {
			return bytes.size();
		}
		const LayoutElement& GetLayout() const noexcept {
			return *pLayout;
		}
		std::shared_ptr<LayoutElement> CloneLayout() const {
			return pLayout;
		}
	private:
		std::shared_ptr<Struct> pLayout;
		std::vector<char> bytes;
	};

	template<typename T>
	LayoutElement& LayoutElement::Add(const std::string& key) noxnd {
		auto ps = dynamic_cast<Struct*>(this);
		assert(ps != nullptr);
		ps->Add(key, std::make_unique<T>());
		return *this;
	}

	template<typename T>
	LayoutElement& LayoutElement::Set(size_t size) noxnd {
		auto pa = dynamic_cast<Array*>(this);
		assert(pa != nullptr);
		pa->Add(std::make_unique<T>(), size);
		return *this;
	}
}