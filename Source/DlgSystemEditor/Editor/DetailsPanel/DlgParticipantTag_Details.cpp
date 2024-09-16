// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgParticipantTag_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"

#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"

#include "DlgSystem/DlgParticipantTag.h"
#include "DlgSystem/DlgManager.h"

#define LOCTEXT_NAMESPACE "DialogueParticipantTag_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNYWhistlerComponentTagCustomization
void FDlgParticipantTag_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
}

void FDlgParticipantTag_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// ParticipantTag
	{
		ParticipantTagPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgParticipantTag, ParticipantTag)).ToSharedRef());
	}
}


TArray<FGameplayTag> FDlgParticipantTag_Details::GetAllParticipantTags() const
{
	return UDlgManager::GetDialoguesParticipantTags();
}

#undef LOCTEXT_NAMESPACE
