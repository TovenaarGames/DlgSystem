// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgGraphNode_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "DlgSystem/DlgConstants.h"
#include "DlgSystem/Nodes/DlgNode.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystem/Nodes/DlgNode_Selector.h"
#include "DlgSystem/Nodes/DlgNode_Proxy.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgMultiLineEditableTextBox_CustomRowHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgObject_CustomRowHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgIntTextBox_CustomRowHelper.h"

#include "Widgets/Input/SButton.h"
#include "PropertyCustomizationHelpers.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTable.h"

#define LOCTEXT_NAMESPACE "DialoguGraphNode_Details"

void FDlgGraphNode_Details::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	// Only support one object being customized
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	TWeakObjectPtr<UDialogueGraphNode> WeakGraphNode = Cast<UDialogueGraphNode>(ObjectsBeingCustomized[0].Get());
	if (!WeakGraphNode.IsValid())
	{
		return;
	}

	GraphNode = WeakGraphNode.Get();
	if (!IsValid(GraphNode))
	{
		return;
	}

	DetailLayoutBuilder = &DetailBuilder;
	Dialogue = GraphNode->GetDialogue();
	const UDlgNode& DialogueNode = GraphNode->GetDialogueNode();
	const bool bIsRootNode = GraphNode->IsRootNode();
	const bool bIsEndNode = GraphNode->IsEndNode();
	const bool bIsSpeechNode = GraphNode->IsSpeechNode();
	const bool bIsSelectorNode = GraphNode->IsSelectorNode();;
	const bool bIsProxyNode = GraphNode->IsProxyNode();;
	const bool bIsSpeechSequenceNode = GraphNode->IsSpeechSequenceNode();
	const bool bIsVirtualParentNode = GraphNode->IsVirtualParentNode();
	const bool bIsCustomNode = GraphNode->IsCustomNode();

	// Hide the existing category
	DetailLayoutBuilder->HideCategory(UDialogueGraphNode::StaticClass()->GetFName());

	// Fill with the properties of the DialogueNode
	IDetailCategoryBuilder& BaseDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Base Node"), FText::GetEmpty(), ECategoryPriority::Important);
	BaseDataCategory.InitiallyCollapsed(false);
	const TSharedPtr<IPropertyHandle> PropertyDialogueNode =
		DetailLayoutBuilder->GetProperty(UDialogueGraphNode::GetMemberNameDialogueNode(), UDialogueGraphNode::StaticClass());

	if (!bIsRootNode)
	{
		// NodeIndex
		BaseDataCategory.AddProperty(UDialogueGraphNode::GetMemberNameNodeIndex(), UDialogueGraphNode::StaticClass());

		// OwnerTag
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameOwnerTag()));

		// OwnerTag
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameOwnerName()));

		// End Nodes and Proxy Nodes can't have children
		if (!bIsEndNode && !bIsProxyNode)
		{
			BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameCheckChildrenOnEvaluation()));
		}

		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterConditions()))
			.ShouldAutoExpand(true);

		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterRestriction()));

		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterEvents()))
			.ShouldAutoExpand(true);
	}

	// GUID
	BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameGUID()))
		.ShouldAutoExpand(true);

	if (GraphNode->CanHaveOutputConnections())
	{
		ChildrenPropertyRow = &BaseDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameChildren())
		);
		ChildrenPropertyRow->ShouldAutoExpand(true);
		ChildrenPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetChildrenVisibility));
	}

	// Do nothing
	if (bIsRootNode)
	{
		return;
	}

	if (bIsCustomNode)
	{
		IDetailCategoryBuilder& CustomCategory = DetailLayoutBuilder->EditCategory(Cast<UDlgNode_Custom>(&DialogueNode)->GetCategoryName());
		CustomCategory.InitiallyCollapsed(false);

		for (FProperty* Property = DialogueNode.GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			if (Property->GetOwnerClass() == UDlgNode_Custom::StaticClass() || Property->GetOwnerClass() == UDlgNode::StaticClass())
			{
				return;
			}
			TSharedPtr<IPropertyHandle> PropertyHandle = PropertyDialogueNode->GetChildHandle(Property->GetFName());
			if (PropertyHandle.IsValid() && PropertyHandle->IsValidHandle())
			{
				CustomCategory.AddProperty(PropertyHandle);
			}
		}

		return;
	}

	// NOTE: be careful here with the child handle names that are common. For example 'Text' so that we do not get the child
	// property from some inner properties
	if (bIsSpeechNode)
	{
		IDetailCategoryBuilder& SpeechCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Node"));
		SpeechCategory.InitiallyCollapsed(false);

		// bIsVirtualParent
		IsVirtualParentPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameIsVirtualParent());
		IsVirtualParentPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &Self::OnIsVirtualParentChanged));
		SpeechCategory.AddProperty(IsVirtualParentPropertyHandle);

		// bVirtualParentFireDirectChildEnterEvents
		if (bIsVirtualParentNode)
		{
			SpeechCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVirtualParentFireDirectChildEnterEvents()))
				.ShouldAutoExpand(true);
		}

		// Text
		{
			TextPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameText());
			FDetailWidgetRow* DetailWidgetRow = &SpeechCategory.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

			TextPropertyRow = MakeShared<FDlgMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
			TextPropertyRow->SetPropertyUtils(DetailBuilder.GetPropertyUtilities());
			TextPropertyRow->Update();

			TextPropertyRow->OnTextCommittedEvent().AddRaw(this, &Self::HandleTextCommitted);
			TextPropertyRow->OnTextChangedEvent().AddRaw(this, &Self::HandleTextChanged);
		}

		// Text arguments
		SpeechCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameTextArguments()))
			.ShouldAutoExpand(true);

		//
		// Data
		//

		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Node Data"));
		SpeechDataCategory.InitiallyCollapsed(false);

		// Speaker State
		{
			const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
				PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameSpeakerState());

			FDetailWidgetRow* DetailWidgetRow = &SpeechDataCategory.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

			SpeakerStatePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
			SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
				SNew(SDlgTextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetDialoguesSpeakerStates)
				.OnTextCommitted(this, &Self::HandleSpeakerStateCommitted)
				.HasContextCheckbox(false)
			)
			.SetVisibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetSpeakerStateNodeVisibility))
			.Update();
		}

		// Node Data that can be anything set by the user
		NodeDataPropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameNodeData())
		);
		NodeDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetNodeDataVisibility));
		NodeDataPropertyRow->ShouldAutoExpand(true);
		NodeDataPropertyRow->ShowPropertyButtons(true);

		// Add Custom buttons
		NodeDataPropertyRow_CustomDisplay = MakeShared<FDlgObject_CustomRowHelper>(NodeDataPropertyRow);
		NodeDataPropertyRow_CustomDisplay->Update();

		// SoundWave
		VoiceSoundWavePropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceSoundWave())
		);
		VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetVoiceSoundWaveVisibility));

		// DialogueWave
		VoiceDialogueWavePropertyRow =  &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceDialogueWave())
		);
		VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetVoiceDialogueWaveVisibility));

		// Generic Data, can be FMOD sound
		GenericDataPropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameGenericData())
		);
		GenericDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetNodeGenericDataVisibility));
	}
	else if (bIsSelectorNode)
	{
		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Selector Node"));
		SpeechDataCategory.InitiallyCollapsed(false);
		SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Selector::GetMemberNameSelectorType()));

		SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Selector::GetMemberNameAvoidPickingSameOptionTwiceInARow()));
		SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Selector::GetMemberNameCycleThroughSatisfiedOptionsWithoutRepetition()));
	}
	else if (bIsProxyNode)
	{
		IDetailCategoryBuilder& ProxyDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Proxy Node"));
		ProxyDataCategory.InitiallyCollapsed(false);

		TSharedPtr<IPropertyHandle> NodeIndexPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Proxy::GetMemberNameNodeIndex());
		FDetailWidgetRow* DetailWidgetRow = &ProxyDataCategory.AddCustomRow(LOCTEXT("TargetNodeSearchKey", "Target Node"));
		NodeIndexPropertyRow = MakeShared<FDlgIntTextBox_CustomRowHelper>(DetailWidgetRow, NodeIndexPropertyHandle, Dialogue);
		NodeIndexPropertyRow->Update();
		FDlgDetailsPanelUtils::SetNumericPropertyLimits<int32>(NodeIndexPropertyHandle, 0, Dialogue->GetNodes().Num() - 1);
	}
	else if (bIsSpeechSequenceNode)
	{
		IDetailCategoryBuilder& SpeechSequenceDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Sequence Node"));
		SpeechSequenceDataCategory.InitiallyCollapsed(false)
			.RestoreExpansionState(true);

		// Add a speech sequence generation tool
		SpeechSequenceDataCategory.AddCustomRow(FText::FromName(GET_MEMBER_NAME_CHECKED(FDlgGraphNode_Details, StringTableSource)))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(FText::FromString("String Table"))
					.TextStyle(FAppStyle::Get(), "SmallText")
					.ToolTipText(FText::FromString("The string table to source the text from."))
			]
			.ValueContent()
			[
				SNew(SObjectPropertyEntryBox)
					.AllowedClass(UStringTable::StaticClass())
					.DisplayBrowse(true)
					.EnableContentPicker(true)
					.ObjectPath_Lambda([this]()
						{
							if (StringTableSource)
							{
								return StringTableSource->GetPathName();
							}
							return FString();
						})
					.OnObjectChanged_Lambda([this](const FAssetData& InAssetData)
						{
							StringTableSource = Cast<UStringTable>(InAssetData.GetAsset());
						})
			];

		SpeechSequenceDataCategory.AddCustomRow(FText::FromName(GET_MEMBER_NAME_CHECKED(FDlgGraphNode_Details, FromStringTableLineID)))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(FText::FromString("From Line ID"))
					.TextStyle(FAppStyle::Get(), "SmallText")
					.ToolTipText(FText::FromString("The ID of the first line in the String table to generate speech sequences from."))
			]
			.ValueContent()
			[
				SNew(SEditableTextBox)
					.OnTextChanged_Lambda([this](const FText& InValue)
						{
							FromStringTableLineID = InValue;
						})
			];

		SpeechSequenceDataCategory.AddCustomRow(FText::FromName(GET_MEMBER_NAME_CHECKED(FDlgGraphNode_Details, ToStringTableLineID)))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(FText::FromString("To Line ID"))
					.TextStyle(FAppStyle::Get(), "SmallText")
					.ToolTipText(FText::FromString("The ID of the last line in the String table to generate speech sequences from."))
			]
			.ValueContent()
			[
				SNew(SEditableTextBox)
					.OnTextChanged_Lambda([this](const FText& InValue)
						{
							ToStringTableLineID = InValue;
						})
			];

		SpeechSequenceDataCategory.AddCustomRow(FText::FromString("Generate Speech Sequence Button"))
			.NameContent()
			[
				SNew(STextBlock)
			]
			.ValueContent()
			[
				SNew(SButton)
					.Text(FText::FromString("Generate"))
					.OnClicked(this, &FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked)
			];

		// Add Speech sequences
		SpeechSequenceDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_SpeechSequence::GetMemberNameSpeechSequence()))
			.ShouldAutoExpand(true);;
	}
}

void FDlgGraphNode_Details::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	// Text arguments already handled in node post change properties
}

void FDlgGraphNode_Details::HandleTextChanged(const FText& InText)
{
	if (GraphNode)
	{
		GraphNode->GetMutableDialogueNode()->RebuildTextArgumentsFromPreview(InText);
	}
}


bool FDlgGraphNode_Details::ParseDialogueStringTableKey(const FString& InKey, FString& OutIdentifier, int32& OutSequenceNumber, FString& OutParticipantName) const
{
	TArray<FString> InKeyArray;
	InKey.ParseIntoArray(InKeyArray, TEXT("_"));

	if (InKeyArray.Num() < 3)
	{
		return false;
	}
	OutParticipantName = InKeyArray.Pop();
	OutSequenceNumber = FCString::Atoi(*InKeyArray.Pop());
	OutIdentifier = FString::Join(InKeyArray, TEXT("_"));
	return true;
}


FString FDlgGraphNode_Details::ComposeDialogueStringTableKey(const FString& Identifier, int32 SequenceNumber, const FString& ParticipantName) const
{
	TArray<FString> StringTableKeyArray;
	if (!Identifier.IsEmpty())
	{
		StringTableKeyArray.Add(Identifier);
	}
	if (SequenceNumber >= 0)
	{
		StringTableKeyArray.Add(FString::FromInt(SequenceNumber));
	}
	if (!ParticipantName.IsEmpty())
	{
		StringTableKeyArray.Add(ParticipantName);
	}

	return FString::Join(StringTableKeyArray, TEXT("_"));
}


FGameplayTag FDlgGraphNode_Details::CreateSpeakerTagFromName(const FString& ParticipantName) const
{
	// Create gameplay tag array containing all possible participant tags
	const FGameplayTagContainer& DialogueCategories = UGameplayTagsManager::Get().RequestGameplayTagChildren(TAG_Dlg);
	FGameplayTagContainer DialogueParticipants;
	for (const FGameplayTag& ParticipantCategory : DialogueCategories)
	{
		DialogueParticipants.AppendTags(UGameplayTagsManager::Get().RequestGameplayTagChildren(ParticipantCategory));
	}
	TArray<FGameplayTag> DialogueParticipantTagArray = DialogueParticipants.GetGameplayTagArray();

	// Early return if array is empty
	if (DialogueParticipantTagArray.IsEmpty())
	{
		return FGameplayTag::EmptyTag;
	}

	// Sort the array according to string similarity with the participant name
	DialogueParticipantTagArray.Sort([&](const FGameplayTag& TagA, const FGameplayTag& TagB)
		{
			FString TagALeaf = TagA.ToString();
			TagALeaf.RemoveFromStart(TagA.RequestDirectParent().ToString());
			FString TagBLeaf = TagB.ToString();
			TagBLeaf.RemoveFromStart(TagB.RequestDirectParent().ToString());
			return CalculateLevenshteinDistance(TagALeaf, ParticipantName) < CalculateLevenshteinDistance(TagBLeaf, ParticipantName);
		});

	// Return the most similar tag
	return DialogueParticipantTagArray[0];
}


int32 FDlgGraphNode_Details::CalculateLevenshteinDistance(const FString& FromString, const FString& ToString) const
{
	// Create a distance matrix containing the distances of the strings from each other
	const int32 FromSize = FromString.Len();
	const int32 ToSize = ToString.Len();
	TArray<TArray<int32>> DistanceMatrix;
	for (int32 i = 0; i <= FromSize; ++i)
	{
		DistanceMatrix.Add(TArray<int32>());
		for (int32 j = 0; j <= ToSize; ++j)
		{
			DistanceMatrix[i].Add(0);
		}
	}

	// If one of the words has zero length, the distance is equal to the size of the other word.
	if (FromSize == 0)
	{
		return ToSize;
	}
	if (ToSize == 0)
	{
		return FromSize;
	}

	// Sets the first row and the first column of the distance matrix with the numerical order from 0 to the length of each word.
	for (int32 i = 0; i <= FromSize; ++i)
	{
		DistanceMatrix[i][0] = i;
	}
	for (int32 j = 0; j <= ToSize; ++j)
	{
		DistanceMatrix[0][j] = j;
	}

	// Verification step / matrix filling.
	for (int32 i = 1; i <= FromSize; ++i)
	{
		for (int32 j = 1; j <= ToSize; ++j)
		{
			// Sets the modification cost.
			// 0 means no modification (i.e. equal letters) and 1 means that a modification is needed (i.e. unequal letters).
			int32 cost = (ToString[j - 1] == FromString[i - 1]) ? 0 : 1;

			// Sets the current position of the matrix as the minimum value between a (deletion), b (insertion) and c (substitution).
			// a = the upper adjacent value plus 1: verif[i - 1][j] + 1
			// b = the left adjacent value plus 1: verif[i][j - 1] + 1
			// c = the upper left adjacent value plus the modification cost: verif[i - 1][j - 1] + cost
			DistanceMatrix[i][j] = FMath::Min(
				FMath::Min(DistanceMatrix[i - 1][j] + 1, DistanceMatrix[i][j - 1] + 1),
				DistanceMatrix[i - 1][j - 1] + cost
			);
		}
	}

	// The last position of the matrix will contain the Levenshtein distance.
	return DistanceMatrix[FromSize][ToSize];
}


void FDlgGraphNode_Details::OnIsVirtualParentChanged()
{
	DetailLayoutBuilder->ForceRefreshDetails();
}


FReply FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked()
{
	// Validate input
	if (!StringTableSource)
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: No string table given!"));
		return FReply::Unhandled();
	}
	const FStringTable& StringTableStruct = StringTableSource->GetStringTable().Get();

	if (!StringTableStruct.FindEntry(FTextKey(FromStringTableLineID.ToString())))
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: %s not found in string table %s!"), *FromStringTableLineID.ToString(), *StringTableSource->GetName());
		return FReply::Unhandled();
	}

	if (!StringTableStruct.FindEntry(FTextKey(ToStringTableLineID.ToString())))
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: %s not found in string table %s!"), *ToStringTableLineID.ToString(), *StringTableSource->GetName());
		return FReply::Unhandled();
	}

	int32 FromSequenceNumber;
	int32 ToSequenceNumber;
	FString DialogueIdentifier;
	FString OtherDialogueIdentier;
	FString DummyParticipant;
	if (!ParseDialogueStringTableKey(FromStringTableLineID.ToString(), DialogueIdentifier, FromSequenceNumber, DummyParticipant))
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: %s does not have the expected 'DialogueID_SequenceNumber_Participant' format!"), *FromStringTableLineID.ToString());
		return FReply::Unhandled();
	}
	if (!ParseDialogueStringTableKey(ToStringTableLineID.ToString(), OtherDialogueIdentier, ToSequenceNumber, DummyParticipant))
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: %s does not have the expected 'DialogueID_SequenceNumber_Participant' format!"), *ToStringTableLineID.ToString());
		return FReply::Unhandled();
	}

	if (DialogueIdentifier != OtherDialogueIdentier)
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: first DialogueID %s does not match last DialogueID %s!"), *DialogueIdentifier, *OtherDialogueIdentier);
		return FReply::Unhandled();
	}
	if (FromSequenceNumber > ToSequenceNumber)
	{
		UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: first SequenceNumber %i needs to be lower or equal to last SequenceNumber %i!"), FromSequenceNumber, ToSequenceNumber);
		return FReply::Unhandled();
	}

	// Clear existing dialogue speech
	const TSharedPtr<IPropertyHandle> PropertyDialogueNode = DetailLayoutBuilder->GetProperty(UDialogueGraphNode::GetMemberNameDialogueNode(), UDialogueGraphNode::StaticClass());
	TSharedPtr<IPropertyHandle> SpeechSequenceProperty = PropertyDialogueNode->GetChildHandle(UDlgNode_SpeechSequence::GetMemberNameSpeechSequence());
	TSharedPtr<IPropertyHandleArray> SpeechSequencePropertyArray = SpeechSequenceProperty->AsArray();
	SpeechSequencePropertyArray->EmptyArray();

	// Populate the array with data from the string table
	TArray<FString> Keys;
	TArray<FString> ParticipantNames;
	for (int32 i = FromSequenceNumber, i_max = ToSequenceNumber + 1; i < i_max; ++i)
	{
		StringTableStruct.EnumerateKeysAndSourceStrings([&](const FTextKey& Key, const FString& SourceString)
			{
				FString KeyString = Key.ToString();
				FString KeyIdentifier;
				FString ParticipantName;
				int32 SequenceNumber;
				if (ParseDialogueStringTableKey(KeyString, KeyIdentifier, SequenceNumber, ParticipantName))
				{
					if (KeyIdentifier == DialogueIdentifier && SequenceNumber == i)
					{
						Keys.Add(KeyString);
						ParticipantNames.Add(ParticipantName);
					}
					return true;
				}
				return false;
			});
	}

	TMap<FString, FGameplayTag> ParticipantNameTagMap;
	for (int32 i = 0, i_max = Keys.Num(); i < i_max; ++i)
	{
		// Get properties from generated arrays
		FString Key = Keys[i];
		FString ParticipantName = ParticipantNames[i];

		// Use existing tag if one is already found for the participant
		FGameplayTag SpeakerTag = ParticipantNameTagMap.Contains(ParticipantName) ? ParticipantNameTagMap.FindRef(ParticipantName) : CreateSpeakerTagFromName(ParticipantName);

		// Add a speech sequence property to the details panel array
		SpeechSequencePropertyArray->AddItem();
		TSharedRef<IPropertyHandle> SpeechSequencePropertyEntry = SpeechSequencePropertyArray->GetElement(i);

		// Fill in the properties
		SpeechSequencePropertyEntry->GetChildHandle(FDlgSpeechSequenceEntry::GetMemberNameText())->SetValue(FText::FromStringTable(StringTableSource->GetStringTableId(), Key));
		if (SpeakerTag.IsValid())
		{
			// Cache valid speaker tag
			ParticipantNameTagMap.Add(ParticipantName, SpeakerTag);

			// Set tag property
			TArray<void*> RawDataPtrs;
			SpeechSequencePropertyEntry->GetChildHandle(FDlgSpeechSequenceEntry::GetMemberNameSpeakerTag())->AccessRawData(RawDataPtrs);
			for (void* RawPtr : RawDataPtrs)
			{
				static_cast<FGameplayTag*>(RawPtr)->FromExportString(SpeakerTag.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FDlgGraphNode_Details::OnGenerateSpeechSequenceButtonClicked: Can't parse a ParticipantTag from SpeakerName %s!"), *ParticipantName);
		}
	}

	// Set owner tag with first tag of dialogue
	if (!ParticipantNameTagMap.IsEmpty() && !ParticipantNames.IsEmpty())
	{
		TArray<void*> RawDataPtrs;
		PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameOwnerTag())->AccessRawData(RawDataPtrs);
		for (void* RawPtr : RawDataPtrs)
		{
			static_cast<FGameplayTag*>(RawPtr)->FromExportString(ParticipantNameTagMap.FindRef(ParticipantNames[0]).ToString());
		}
	}

	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
