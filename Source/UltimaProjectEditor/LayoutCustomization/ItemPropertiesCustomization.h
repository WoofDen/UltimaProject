#pragma once

#include "IPropertyTypeCustomization.h"

class FItemPropertiesCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FItemPropertiesCustomization>();
	}

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	                             FDetailWidgetRow& HeaderRow,
	                             IPropertyTypeCustomizationUtils& Utils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	                               IDetailChildrenBuilder& ChildBuilder,
	                               IPropertyTypeCustomizationUtils& Utils) override;
};
