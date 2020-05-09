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