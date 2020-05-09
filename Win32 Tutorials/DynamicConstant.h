#pragma once
#include "ConditionalNoexcept.h"
#include <cassert>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <numeric>

#define DCB_RESOLVE_BASE(eltype) \
virtual size_t Resolve ## eltype() const noxnd;

#define DCB_LEAF_ELEMENT_IMPL(eltype,systype,hlslSize) \
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
		virtual LayoutElement& operator[](const std::string&);
		virtual const LayoutElement& operator[](const std::string&) const;
		//T() only works for arrays
		virtual LayoutElement& T();
		virtual const LayoutElement& T() const;
		//offset based -only works after finalization
		size_t GetOffsetBegin() const noexcept;
		virtual size_t GetOffsetEnd() const noexcept = 0;
		//Gets size in bytes derived from offsets
		size_t GetSizeInBytes() const noexcept;

		//Only works for structs
		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd;
		//Only works for arrays
		template<typename T>
		LayoutElement& Set(size_t size) noxnd;

		//Returns the value of the offset up to the next 16byte boundary
		static size_t GetNextBoundaryOffset(size_t offset);

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
		LayoutElement& operator[](const std::string& key) override final;
		const LayoutElement& operator[](const std::string& key) const override final;
		size_t GetOffsetEnd() const noexcept override final;
		void Add(const std::string& name, std::unique_ptr<LayoutElement> pElement) noxnd;
	protected:
		size_t Finalize(size_t offset_in) override final;
		size_t ComputeSize() const noxnd override final;
	private:
		static size_t CalculatePaddingBeforeElement(size_t offset, size_t size) noexcept;
	private:
		std::unordered_map<std::string,LayoutElement*> map;
		std::vector<std::unique_ptr<LayoutElement>> elements;
	};

	class Array : public LayoutElement {
	public:
		size_t GetOffsetEnd() const noexcept override final;
		void Set(std::unique_ptr<LayoutElement> pElement, size_t size_in) noxnd;
		LayoutElement& T() override final;
		const LayoutElement& T() const override final;
	protected:
		size_t Finalize(size_t offset_in) override final;
		size_t ComputeSize() const noxnd override final;
	private:
		size_t size = 0u;
		std::unique_ptr<LayoutElement> pElement;
	};

	class Layout {
	public:
		Layout() : pLayout(std::make_shared<Struct>()) {}

		LayoutElement& operator[](const std::string& key);
		Layout(std::shared_ptr<LayoutElement> pLayout)
			: pLayout(std::move(pLayout)) {}

		size_t GetSizeInBytes() const noexcept;
		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd {
			assert(!finalized && "Cannot modify finalized layout");
			return pLayout->Add<T>(key);
		}
		std::shared_ptr<LayoutElement> Finalize();

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

		ConstElementRef operator[](const std::string& key) noxnd;
		ConstElementRef operator[](int index) noxnd;

		Ptr operator&() noxnd;

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

		operator ConstElementRef() const noexcept;

		ElementRef operator[](const std::string& key) noxnd;
		ElementRef operator[](size_t index) noxnd;
		Ptr operator&() noxnd;
		
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

		ElementRef operator[](const std::string& key) noxnd;
		ConstElementRef operator[](const std::string& key) const noxnd;
		const char* GetData() const noexcept;
		size_t GetSizeInBytes() const noexcept;
		const LayoutElement& GetLayout() const noexcept;
		std::shared_ptr<LayoutElement> CloneLayout() const;
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
		pa->Set(std::make_unique<T>(), size);
		return *this;
	}
}

#undef DCB_RESOLVE_BASE
#undef DCB_LEAF_ELEMENT_IMPL
#undef DCB_LEAF_ELEMENT
#undef DCB_REF_CONVERSION
#undef DCB_REF_ASSIGN
#undef DCB_REF_NONCONST
#undef DCB_REF_CONST
#undef DCB_PTR_CONVERSION