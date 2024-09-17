// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_SpeechSequence.h"

#include "DlgSystem/DlgContext.h"
#include "DlgSystem/DlgLocalizationHelper.h"
#include "DlgSystem/DlgHelper.h"


#if WITH_EDITOR
void UDlgNode_SpeechSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	InitializeNodeDataOnArrayAdd(PropertyChangedEvent);

	// fill edges automatically based on input data
	AutoGenerateInnerEdges();

	// Rebuild text arguments
	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const bool bSpeechSequnceChanged = PropertyName == GetMemberNameSpeechSequence();
	if (bSpeechSequnceChanged)
	{
		RebuildTextArguments(true);
	}
}

void UDlgNode_SpeechSequence::InitializeNodeDataOnArrayAdd(FPropertyChangedEvent& PropertyChangedEvent)
{
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	if (Settings == nullptr || Settings->DefaultCustomNodeDataClass.IsNull())
	{
		return;
	}

	if (PropertyChangedEvent.GetPropertyName() != GetMemberNameSpeechSequence() ||
		PropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayAdd)
	{
		return;
	}

	int32 Index = PropertyChangedEvent.GetArrayIndex(GetMemberNameSpeechSequence().ToString());
	if (Index == INDEX_NONE)
	{
		return;
	}

	check(SpeechSequence.IsValidIndex(Index));
	SpeechSequence[Index].NodeData = NewObject<UDlgNodeData>(this, Settings->DefaultCustomNodeDataClass.Get(), NAME_None, GetMaskedFlags(RF_PropagateToSubObjects), NULL);
}
#endif

void UDlgNode_SpeechSequence::UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode)
{
	for (FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		// We only care about edges here
		if (Settings.bSetDefaultEdgeTexts)
		{
			// Inner edges always point to a normal node and are always the unique edge child
			if (Entry.EdgeText.IsEmpty())
			{
				Entry.EdgeText = Settings.DefaultTextEdgeToNormalNode;
			}
		}

		Entry.UpdateTextsValuesFromDefaultsAndRemappings(Settings);
	}
	Super::UpdateTextsValuesFromDefaultsAndRemappings(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_SpeechSequence::UpdateTextsNamespacesAndKeys(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode)
{
	UObject* Outer = GetOuter();
	if (!IsValid(Outer))
	{
		return;
	}

	for (FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		Entry.UpdateTextsNamespacesAndKeys(Outer, Settings);
	}

	Super::UpdateTextsNamespacesAndKeys(Settings, bEdges, bUpdateGraphNode);
}

bool UDlgNode_SpeechSequence::HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	ActualIndex = 0;

	RebuildConstructedText(Context);

	return Super::HandleNodeEnter(Context, NodesEnteredWithThisStep);
}

bool UDlgNode_SpeechSequence::ReevaluateChildren(UDlgContext& Context, TSet<const UDlgNode*> AlreadyEvaluated)
{
	TArray<FDlgEdge>& Options = Context.GetMutableOptionsArray();
	TArray<FDlgEdgeData>& AllOptions = Context.GetAllMutableOptionsArray();
	Options.Empty();
	AllOptions.Empty();

	// If the last entry is active the real edges are used
	if (ActualIndex == SpeechSequence.Num() - 1)
		return Super::ReevaluateChildren(Context, AlreadyEvaluated);

	// give the context the fake inner edge
	if (InnerEdges.IsValidIndex(ActualIndex))
	{
		Options.Add(InnerEdges[ActualIndex]);
		AllOptions.Add(FDlgEdgeData{ true, InnerEdges[ActualIndex] });
		return true;
	}

	return false;
}

bool UDlgNode_SpeechSequence::OptionSelected(int32 OptionIndex, bool bFromAll, UDlgContext& Context)
{
	// Actual index is valid, and not the last node in the speech sequence, increment
	if (ActualIndex >= 0 && ActualIndex < SpeechSequence.Num() - 1)
	{
		ActualIndex += 1;
		return ReevaluateChildren(Context, {this});
	}

	// node finished -> generate true children
	ActualIndex = 0;
	Super::ReevaluateChildren(Context, { this });
	return Super::OptionSelected(OptionIndex, bFromAll, Context);
}

void UDlgNode_SpeechSequence::RebuildConstructedText(const UDlgContext& Context)
{
	for (FDlgSpeechSequenceEntry& entry : SpeechSequence)
	{
		entry.RebuildConstructedText(Context, GetNodeParticipantTag());
	}	
}

void UDlgNode_SpeechSequence::RebuildTextArguments(bool bEdges, bool bUpdateGraphNode)
{
	_TextArguments.Empty(); // Also refresh array with all entries' text arguments.
	for (FDlgSpeechSequenceEntry& entry : SpeechSequence)
	{
		entry.RebuildTextArguments();
		_TextArguments.Append(entry.GetTextArguments());
	}

	Super::RebuildTextArguments(bEdges, bUpdateGraphNode);
}

bool UDlgNode_SpeechSequence::OptionSelectedFromReplicated(int32 OptionIndex, bool bFromAll, UDlgContext& Context)
{
	// Is the new option index valid? set that for the actual index
	if (SpeechSequence.IsValidIndex(OptionIndex))
	{
		ActualIndex = OptionIndex;
		return ReevaluateChildren(Context, { this });
	}

	// node finished -> generate true children
	ActualIndex = 0;
	Super::ReevaluateChildren(Context, { this });
	return Super::OptionSelected(OptionIndex, bFromAll, Context);
}

const FText& UDlgNode_SpeechSequence::GetNodeText() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].GetNodeText();
	}

	return FText::GetEmpty();
}

UDlgNodeData* UDlgNode_SpeechSequence::GetNodeData() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].NodeData;
	}

	return nullptr;
}

USoundBase* UDlgNode_SpeechSequence::GetNodeVoiceSoundBase() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].VoiceSoundWave;
	}

	return nullptr;
}

UDialogueWave* UDlgNode_SpeechSequence::GetNodeVoiceDialogueWave() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].VoiceDialogueWave;
	}

	return nullptr;
}

UObject* UDlgNode_SpeechSequence::GetNodeGenericData() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].GenericData;
	}

	return nullptr;
}

FName UDlgNode_SpeechSequence::GetSpeakerState() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].SpeakerState;
	}

	return NAME_None;
}

void UDlgNode_SpeechSequence::AddAllSpeakerStatesIntoSet(TSet<FName>& OutStates) const
{
	for (const auto& SpeechEntry : SpeechSequence)
	{
		OutStates.Add(SpeechEntry.SpeakerState);
	}
}

FGameplayTag UDlgNode_SpeechSequence::GetNodeParticipantTag() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].SpeakerTag;
	}

	return OwnerTag;
}

void UDlgNode_SpeechSequence::GetAssociatedParticipants(TArray<FGameplayTag>& OutArray) const
{
	Super::GetAssociatedParticipants(OutArray);

	for (const FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		Entry.GetAssociatedParticipants(OutArray);
	}
}

void UDlgNode_SpeechSequence::AutoGenerateInnerEdges()
{
	InnerEdges.Empty();
	for (const FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		FDlgEdge Edge;
		Edge.SetUnformattedText(Entry.EdgeText);
		InnerEdges.Add(Edge);
	}
}

void FDlgSpeechSequenceEntry::SetNodeText(const FText& InText, const TArray<FDlgTextArgument>& InArguments)
{
	Text = InText;
	if (TextArguments.Num() == 0 && InArguments.Num() > 0)
	{
		TextArguments = InArguments;
	}

	RebuildTextArguments();
}

const FText& FDlgSpeechSequenceEntry::GetNodeText() const
{
	if (TextArguments.Num() > 0 && !ConstructedText.IsEmpty())
	{
		return ConstructedText;
	}
	return Text;
}

void FDlgSpeechSequenceEntry::RebuildConstructedText(const UDlgContext& Context, const FGameplayTag& OwnerTag)
{
	if (TextArguments.Num() <= 0)
	{
		return;
	}

	FFormatNamedArguments OrderedArguments;
	for (const FDlgTextArgument& DlgArgument : TextArguments)
	{
		OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(Context, OwnerTag));
	}
	ConstructedText = FText::AsCultureInvariant(FText::Format(Text, OrderedArguments));
}

void FDlgSpeechSequenceEntry::RebuildTextArguments()
{
	FDlgTextArgument::UpdateTextArgumentArray(Text, TextArguments);
}

void FDlgSpeechSequenceEntry::UpdateTextsNamespacesAndKeys(const UObject* Outer, const UDlgSystemSettings& Settings)
{
	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(Outer, Settings, Text);
	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(Outer, Settings, EdgeText);
}

void FDlgSpeechSequenceEntry::UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings& Settings)
{
	FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Text);
	FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, EdgeText);
}

void FDlgSpeechSequenceEntry::GetAssociatedParticipants(TArray<FGameplayTag>& OutArray) const
{
	if (UBSDlgFunctions::IsValidParticipantTag(SpeakerTag))
	{
		OutArray.AddUnique(SpeakerTag);
	}

	for (const FDlgTextArgument& TextArgument : TextArguments)
	{
		if (UBSDlgFunctions::IsValidParticipantTag(TextArgument.ParticipantTag))
		{
			OutArray.AddUnique(TextArgument.ParticipantTag);
		}
	}
}
