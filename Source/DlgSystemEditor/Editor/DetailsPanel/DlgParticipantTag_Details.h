// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "IDetailPropertyRow.h"
#include "GameplayTagContainer.h"

class FDlgTextPropertyPickList_CustomRowHelper;

class DLGSYSTEMEDITOR_API FDlgParticipantTag_Details : public IPropertyTypeCustomization
{
	typedef FDlgParticipantTag_Details Self;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
		IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	/** Gathers selectable options. */
	TArray<FGameplayTag> GetAllParticipantTags() const;

private:
	// Cache the some property handles
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	// Cache the rows of the properties, created in CustomizeChildren
	IDetailPropertyRow* ParticipantTagPropertyRow;
};
