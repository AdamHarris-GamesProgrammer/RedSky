#pragma once
#include "ConditionalNoexcept.h"
#include <cassert>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <unordered_map>


#define DCB_RESOLVE_BASE(eltype) \
virtual size_t Resolve ## eltype() const noxnd;

#define DCB_LEAF_ELEMENT_IMPL(eltype,systype,hlslSize) \
class eltype : public LayoutElement \
{ \
	friend LayoutElement; \
public: \
	using SystemType = systype; \
	size_t Resolve ## eltype() const noxnd  final;\
	size_t GetOffsetEnd() const noexcept  final;\
	std::string GetSignature() const noxnd final; \
protected: \
	size_t Finalize( size_t offset_in ) noxnd final;\
	size_t ComputeSize() const noxnd  final;\
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


namespace Dcb
{
	namespace dx = DirectX;
	class LayoutCodex;
	class LayoutElement
	{
		friend class RawLayout;
		friend class Array;
		friend class Struct;
	public:
		virtual ~LayoutElement();

		virtual std::string GetSignature() const noxnd = 0;

		virtual bool Exists() const noexcept;

		// [] only works for Structs; access member by name
		virtual LayoutElement& operator[](const std::string&) noxnd;
		const LayoutElement& operator[](const std::string& key) const noxnd;
		// T() only works for Arrays; gets the array type layout object
		virtual LayoutElement& T() noxnd;
		const LayoutElement& T() const noxnd;

		// offset based- functions only work after finalization!
		size_t GetOffsetBegin() const noexcept;
		virtual size_t GetOffsetEnd() const noexcept = 0;
		// get size in bytes derived from offsets
		size_t GetSizeInBytes() const noexcept;

		// only works for Structs; add LayoutElement
		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd;
		// only works for Arrays; set the type and the # of elements
		template<typename T>
		LayoutElement& Set(size_t size) noxnd;

		// returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
		static size_t GetNextBoundaryOffset(size_t offset) noexcept;

		DCB_RESOLVE_BASE(Matrix)
			DCB_RESOLVE_BASE(Float4)
			DCB_RESOLVE_BASE(Float3)
			DCB_RESOLVE_BASE(Float2)
			DCB_RESOLVE_BASE(Float)
			DCB_RESOLVE_BASE(Bool)
	protected:
		// sets all offsets for element and subelements, returns offset directly after this element
		virtual size_t Finalize(size_t offset) noxnd = 0;
		// computes the size of this element in bytes, considering padding on Arrays and Structs
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


	class Struct : public LayoutElement
	{
		friend LayoutElement;
	public:
		LayoutElement& operator[](const std::string& key) noxnd final;
		size_t GetOffsetEnd() const noexcept final;
		std::string GetSignature() const noxnd final;
		void Add(const std::string& name, std::unique_ptr<LayoutElement> pElement) noxnd;
	protected:
		Struct() = default;
		size_t Finalize(size_t offset_in) noxnd final;
		size_t ComputeSize() const noxnd final;
	private:
		static size_t CalculatePaddingBeforeElement(size_t offset, size_t size) noexcept;
	private:
		std::unordered_map<std::string, LayoutElement*> map;
		std::vector<std::unique_ptr<LayoutElement>> elements;
	};

	class Array : public LayoutElement
	{
		friend LayoutElement;
	public:
		size_t GetOffsetEnd() const noexcept  final;
		void Set(std::unique_ptr<LayoutElement> pElement, size_t size_in) noxnd;
		LayoutElement& T() noxnd final;
		const LayoutElement& T() const noxnd;
		std::string GetSignature() const noxnd final;
		bool IndexInBounds(size_t index) const noexcept{
			return index < size;
		}
	protected:
		Array() = default;
		size_t Finalize(size_t offset_in) noxnd final;
		size_t ComputeSize() const noxnd final;
	private:
		size_t size = 0u;
		std::unique_ptr<LayoutElement> pElement;
	};

	class Layout {
		friend class LayoutCodex;
		friend class Buffer;

	public:
		size_t GetSizeInBytes() const noexcept;
		std::string GetSignature() const noxnd;
	protected:
		Layout() noexcept;
		Layout(std::shared_ptr<LayoutElement> pRoot) noexcept;
		std::shared_ptr<LayoutElement> pRoot;
	};

	class RawLayout : public Layout {
		friend class LayoutCodex;
	public:
		RawLayout() = default;
		LayoutElement& operator[](const std::string& key) noxnd;

		template<typename T>
		LayoutElement& Add(const std::string& key) noxnd
		{
			return pRoot->Add<T>(key);
		}

	private:
		std::shared_ptr<LayoutElement> DeliverRoot() noexcept;
		void ClearRoot() noexcept;
	};

	class CookedLayout : public Layout {
		friend class LayoutCodex;
		friend class Buffer;
	public:
		const LayoutElement& operator[](const std::string& key) const noxnd;
		std::shared_ptr<LayoutElement> ShareRoot() const noexcept;

	private:
		CookedLayout(std::shared_ptr<LayoutElement> pRoot) noexcept;

	};

	class ConstElementRef
	{
		friend class ElementRef;
		friend class Buffer;
	public:
		class Ptr
		{
			friend ConstElementRef;
		public:

			DCB_PTR_CONVERSION(Matrix, const)
				DCB_PTR_CONVERSION(Float4, const)
				DCB_PTR_CONVERSION(Float3, const)
				DCB_PTR_CONVERSION(Float2, const)
				DCB_PTR_CONVERSION(Float, const)
				DCB_PTR_CONVERSION(Bool, const)
		private:
			Ptr(ConstElementRef& ref) noexcept;
			ConstElementRef& ref;
		};
	public:

		bool Exists() const noexcept;
		ConstElementRef operator[](const std::string& key) noxnd;
		ConstElementRef operator[](size_t index) noxnd;
		Ptr operator&() noxnd;

		DCB_REF_CONST(Matrix)
			DCB_REF_CONST(Float4)
			DCB_REF_CONST(Float3)
			DCB_REF_CONST(Float2)
			DCB_REF_CONST(Float)
			DCB_REF_CONST(Bool)
	private:
		ConstElementRef(const LayoutElement* pLayout, char* pBytes, size_t offset) noexcept;
		size_t offset;
		const class LayoutElement* pLayout;
		char* pBytes;
	};

	class ElementRef
	{
		friend class Buffer;
	public:
		class Ptr
		{
			friend ElementRef;
		public:

			DCB_PTR_CONVERSION(Matrix)
				DCB_PTR_CONVERSION(Float4)
				DCB_PTR_CONVERSION(Float3)
				DCB_PTR_CONVERSION(Float2)
				DCB_PTR_CONVERSION(Float)
				DCB_PTR_CONVERSION(Bool)
		private:
			Ptr(ElementRef& ref) noexcept;
			ElementRef& ref;
		};
	public:
		bool Exists() const noexcept;
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
		ElementRef(const LayoutElement* pLayout, char* pBytes, size_t offset) noexcept;
		size_t offset;
		const class LayoutElement* pLayout;
		char* pBytes;
	};


	class Buffer
	{
	public:
		static Buffer Make(RawLayout&& lay) noxnd;
		static Buffer Make(const CookedLayout& lay) noxnd;
		Buffer(const Buffer&) noexcept;
		ElementRef operator[](const std::string& key) noxnd;
		ConstElementRef operator[](const std::string& key) const noxnd;
		std::string GetSignature() const noxnd;
		const char* GetData() const noexcept;
		size_t GetSizeInBytes() const noexcept;
		const LayoutElement& GetLayout() const noexcept;
		void CopyFrom(const Buffer&) noxnd;
		std::shared_ptr<LayoutElement> ShareLayout() const noexcept;
	private:
		Buffer(const CookedLayout& lay) noexcept;
		std::shared_ptr<LayoutElement> pLayout;
		std::vector<char> bytes;
	};


	// must come after Definitions of Struct and Array
	template<typename T>
	LayoutElement& LayoutElement::Add(const std::string& key) noxnd
	{
		auto ps = dynamic_cast<Struct*>(this);
		assert(ps != nullptr);
		struct Enabler : public T {};
		ps->Add(key, std::make_unique<Enabler>());
		return *this;
	}

	template<typename T>
	LayoutElement& LayoutElement::Set(size_t size) noxnd
	{
		auto pa = dynamic_cast<Array*>(this);
		assert(pa != nullptr);	
		struct Enabler : public T {};
		pa->Set(std::make_unique<Enabler>(), size);
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
