// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSystem/DlgContext.h"

#include "Net/UnrealNetwork.h"
#include "Engine/Texture2D.h"
#include "Engine/Blueprint.h"

#include "DlgConstants.h"
#include "Nodes/DlgNode.h"
#include "Nodes/DlgNode_End.h"
#include "Nodes/DlgNode_SpeechSequence.h"
#include "DlgDialogueParticipant.h"
#include "DlgMemory.h"
#include "Logging/DlgLogger.h"


UDlgContext::UDlgContext(const FObjectInitializer& ObjectInitializer)
	: UDlgObject(ObjectInitializer)
{
	//UObject.bReplicates = true;
}

void UDlgContext::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Dialogue);
	DOREPLIFETIME(ThisClass, SerializedParticipants);
}

void UDlgContext::SerializeParticipants()
{
	SerializedParticipants.Empty(Participants.Num());
	for (const auto& KeyValue : Participants)
	{
		SerializedParticipants.Add(KeyValue.Value);
	}
}

void UDlgContext::OnRep_SerializedParticipants()
{
	Participants.Empty(SerializedParticipants.Num());
	for (UObject* Participant : SerializedParticipants)
	{
		if (IsValid(Participant))
		{
			Participants.Add(IDlgDialogueParticipant::Execute_GetParticipantTag(Participant), Participant);
		}
	}
}

bool UDlgContext::ChooseOption(int32 OptionIndex)
{
	check(Dialogue);
	if (UDlgNode* Node = GetMutableActiveNode())
	{
		if (Node->OptionSelected(OptionIndex, false, *this))
		{
			return true;
		}
	}

	bDialogueEnded = true;
	return false;
}

bool UDlgContext::ChooseSpeechSequenceOptionFromReplicated(int32 OptionIndex)
{
	check(Dialogue);
	if (UDlgNode_SpeechSequence* Node = GetMutableActiveNodeAsSpeechSequence())
	{
		if (Node->OptionSelectedFromReplicated(OptionIndex, false, *this))
		{
			return true;
		}
	}

	bDialogueEnded = true;
	return false;
}

bool UDlgContext::ChooseOptionFromAll(int32 Index)
{
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("ChooseOptionFromAll - INVALID given Index = %d"), Index));
		bDialogueEnded = true;
		return false;
	}

	if (UDlgNode* Node = GetMutableActiveNode())
	{
		if (Node->OptionSelected(Index, true, *this))
		{
			return true;
		}
	}

	bDialogueEnded = true;
	return false;
}

bool UDlgContext::ReevaluateOptions()
{
	check(Dialogue);
	UDlgNode* Node = GetMutableActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("ReevaluateOptions - Failed to update dialogue options"));
		return false;
	}

	return Node->ReevaluateChildren(*this, {});
}

const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionText - INVALID given OptionIndex = %d"), OptionIndex));
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex].GetText();
}

FName UDlgContext::GetOptionSpeakerState(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionSpeakerState - INVALID given OptionIndex = %d"), OptionIndex));
		return NAME_None;
	}

	return AvailableChildren[OptionIndex].SpeakerState;
}

const TArray<FDlgCondition>& UDlgContext::GetOptionEnterConditions(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionEnterConditions - INVALID given OptionIndex = %d"), OptionIndex));
		static TArray<FDlgCondition> EmptyArray;
		return EmptyArray;
	}

	return AvailableChildren[OptionIndex].Conditions;
}

const FDlgEdge& UDlgContext::GetOption(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOption - INVALID given OptionIndex = %d"), OptionIndex));
		return FDlgEdge::GetInvalidEdge();
	}

	return AvailableChildren[OptionIndex];
}

const FText& UDlgContext::GetOptionTextFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionTextFromAll - INVALID given Index = %d"), Index));
		return FText::GetEmpty();
	}

	return AllChildren[Index].GetEdge().GetText();
}

bool UDlgContext::IsOptionSatisfied(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("IsOptionSatisfied - INVALID given Index = %d"), Index));
		return false;
	}

	return AllChildren[Index].IsSatisfied();
}

FName UDlgContext::GetOptionSpeakerStateFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionSpeakerStateFromAll - INVALID given Index = %d"), Index));
		return NAME_None;
	}

	return AllChildren[Index].GetEdge().SpeakerState;
}

const FDlgEdgeData& UDlgContext::GetOptionFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionFromAll - INVALID given Index = %d"), Index));
		return FDlgEdgeData::GetInvalidEdge();
	}

	return AllChildren[Index];
}

const FText& UDlgContext::GetActiveNodeText() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeText - INVALID Active Node"));
		return FText::GetEmpty();
	}

	return Node->GetNodeText();
}

FName UDlgContext::GetActiveNodeSpeakerState() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeSpeakerState - INVALID Active Node"));
		return NAME_None;
	}

	return Node->GetSpeakerState();
}

USoundWave* UDlgContext::GetActiveNodeVoiceSoundWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceSoundWave - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceSoundWave();
}

USoundBase* UDlgContext::GetActiveNodeVoiceSoundBase() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceSoundBase - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceSoundBase();
}

UDialogueWave* UDlgContext::GetActiveNodeVoiceDialogueWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceDialogueWave - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceDialogueWave();
}

UObject* UDlgContext::GetActiveNodeGenericData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeGenericData - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeGenericData();
}

UDlgNodeData* UDlgContext::GetActiveNodeData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeData - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeData();
}

UTexture2D* UDlgContext::GetActiveNodeParticipantIcon() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipantIcon - INVALID Active Node"));
		return nullptr;
	}

	const FGameplayTag& SpeakerTag = Node->GetNodeParticipantTag();
	auto* ObjectPtr = Participants.Find(SpeakerTag);
	if (ObjectPtr == nullptr || !IsValid(*ObjectPtr))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("GetActiveNodeParticipantIcon - The ParticipantTag = `%s` from the Active Node does NOT exist in the current Participants"),
			*SpeakerTag.ToString()
		));
		return nullptr;
	}

	return IDlgDialogueParticipant::Execute_GetParticipantIcon(*ObjectPtr, SpeakerTag, Node->GetSpeakerState());
}

UObject* UDlgContext::GetActiveNodeParticipant() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipant - INVALID Active Node"));
		return nullptr;
	}

	const FGameplayTag& SpeakerTag = Node->GetNodeParticipantTag();
	auto* ObjectPtr = Participants.Find(Node->GetNodeParticipantTag());
	if (ObjectPtr == nullptr || !IsValid(*ObjectPtr))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("GetActiveNodeParticipant - The ParticipantTag = `%s` from the Active Node does NOT exist in the current Participants"),
			*SpeakerTag.ToString()
		));
		return nullptr;
	}

	return *ObjectPtr;
}

FGameplayTag UDlgContext::GetActiveNodeParticipantTag() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipantTag - INVALID Active Node"));
		return FGameplayTag::EmptyTag;
	}

	return Node->GetNodeParticipantTag();
}

FText UDlgContext::GetActiveNodeParticipantDisplayName() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipantDisplayName - INVALID Active Node"));
		return FText::GetEmpty();
	}

	const FGameplayTag& SpeakerTag = Node->GetNodeParticipantTag();
	auto* ObjectPtr = Participants.Find(SpeakerTag);
	if (ObjectPtr == nullptr || !IsValid(*ObjectPtr))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("GetActiveNodeParticipantDisplayName - The ParticipantTag = `%s` from the Active Node does NOT exist in the current Participants"),
			*SpeakerTag.ToString()
		));
		return FText::GetEmpty();
	}

	return IDlgDialogueParticipant::Execute_GetParticipantDisplayName(*ObjectPtr, SpeakerTag);
}

UObject* UDlgContext::GetMutableParticipant(const FGameplayTag& ParticipantTag) const
{
	auto* ParticipantPtr = Participants.Find(ParticipantTag);
	if (ParticipantPtr != nullptr && IsValid(*ParticipantPtr))
	{
		return *ParticipantPtr;
	}

	return nullptr;
}

const UObject* UDlgContext::GetParticipant(const FGameplayTag& ParticipantTag) const
{
	auto* ParticipantPtr = Participants.Find(ParticipantTag);
	if (ParticipantPtr != nullptr && IsValid(*ParticipantPtr))
	{
		return *ParticipantPtr;
	}

	return nullptr;
}

UObject* UDlgContext::GetParticipantFromName(const FDlgParticipantTag& Participant)
{
	if (UObject** ParticipantObjectPtr = Participants.Find(Participant.ParticipantTag))
	{
		return *ParticipantObjectPtr;
	}

	return nullptr;
}

bool UDlgContext::IsValidNodeIndex(int32 NodeIndex) const
{
	return Dialogue ? Dialogue->IsValidNodeIndex(NodeIndex) : false;
}

bool UDlgContext::IsValidNodeGUID(const FGuid& NodeGUID) const
{
	return Dialogue ? Dialogue->IsValidNodeGUID(NodeGUID) : false;
}

FGuid UDlgContext::GetNodeGUIDForIndex(int32 NodeIndex) const
{
	return Dialogue ? Dialogue->GetNodeGUIDForIndex(NodeIndex) : FGuid{};
}

int32 UDlgContext::GetNodeIndexForGUID(const FGuid& NodeGUID) const
{
	return Dialogue ? Dialogue->GetNodeIndexForGUID(NodeGUID) : INDEX_NONE;
}

bool UDlgContext::IsOptionConnectedToVisitedNode(int32 Index, bool bLocalHistory, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToVisitedNode - INVALID Index = %d for AvailableChildren"), Index));
			return false;
		}
		TargetIndex = AvailableChildren[Index].TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToVisitedNode - INVALID Index = %d for AllChildren"), Index));
			return false;
		}
		TargetIndex = AllChildren[Index].GetEdge().TargetIndex;
	}

	const FGuid TargetGUID = GetNodeGUIDForIndex(TargetIndex);
	if (!bLocalHistory && Dialogue == nullptr)
	{
		LogErrorWithContext(TEXT("IsOptionConnectedToVisitedNode - This Context does not have a valid Dialogue"));
		return false;
	}

	return IsNodeVisited(TargetIndex, TargetGUID, bLocalHistory);
}

bool UDlgContext::IsOptionConnectedToEndNode(int32 Index, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - INVALID Index = %d for AvailableChildren"), Index));
			return false;
		}
		TargetIndex = AvailableChildren[Index].TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - INVALID Index = %d for AllChildren"), Index));
			return false;
		}
		TargetIndex = AllChildren[Index].GetEdge().TargetIndex;
	}

	if (Dialogue == nullptr)
	{
		LogErrorWithContext(TEXT("IsOptionConnectedToEndNode - This Context does not have a valid Dialogue"));
		return false;
	}

	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
	if (Nodes.IsValidIndex(TargetIndex))
	{
		return Nodes[TargetIndex]->IsA<UDlgNode_End>();
	}

	LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - The examined Edge/Option at Index = %d does not point to a valid node"), Index));
	return false;
}

bool UDlgContext::EnterNode(int32 NodeIndex, bool bFireEnterEvents, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(Dialogue);
	UDlgNode* Node = GetMutableNodeFromIndex(NodeIndex);
	if (!IsValid(Node))
	{
		LogErrorWithContext(FString::Printf(TEXT("EnterNode - FAILED because of INVALID NodeIndex = %d"), NodeIndex));
		return false;
	}

	ActiveNodeIndex = NodeIndex;
	SetNodeVisited(NodeIndex, Node->GetGUID());

	return Node->HandleNodeEnter(*this, bFireEnterEvents, NodesEnteredWithThisStep);
}

UDlgContext* UDlgContext::CreateCopy() const
{
	UObject* FirstParticipant = nullptr;
	for (const auto& KeyValue : Participants)
	{
		if (KeyValue.Value)
		{
			FirstParticipant = KeyValue.Value;
			break;
		}
	}
	if (!FirstParticipant)
	{
		return nullptr;
	}

	auto* Context = NewObject<UDlgContext>(FirstParticipant, GetClass());
	Context->Dialogue = Dialogue;
	Context->SetParticipants(Participants);
	Context->ActiveNodeIndex = ActiveNodeIndex;
	Context->AvailableChildren = AvailableChildren;
	Context->AllChildren = AllChildren;
	Context->History = History;
	Context->bDialogueEnded = bDialogueEnded;

	return Context;
}

void UDlgContext::SetNodeVisited(int32 NodeIndex, const FGuid& NodeGUID)
{
	FDlgMemory::Get().SetNodeVisited(Dialogue->GetGUID(), NodeIndex, NodeGUID);
	History.Add(NodeIndex, NodeGUID);
}

bool UDlgContext::IsNodeVisited(int32 NodeIndex, const FGuid& NodeGUID, bool bLocalHistory) const
{
	if (bLocalHistory)
	{
		return History.Contains(NodeIndex, NodeGUID);
	}

	return FDlgMemory::Get().IsNodeVisited(Dialogue->GetGUID(), NodeIndex, NodeGUID);
}

FDlgNodeSavedData& UDlgContext::GetNodeSavedData(const FGuid& NodeGUID)
{
	return FDlgMemory::Get().FindOrAddEntry(Dialogue->GetGUID()).GetNodeData(NodeGUID);
}

UDlgNode_SpeechSequence* UDlgContext::GetMutableActiveNodeAsSpeechSequence() const
{
	return Cast<UDlgNode_SpeechSequence>(GetMutableNodeFromIndex(ActiveNodeIndex));
}

const UDlgNode_SpeechSequence* UDlgContext::GetActiveNodeAsSpeechSequence() const
{
	return Cast<UDlgNode_SpeechSequence>(GetNodeFromIndex(ActiveNodeIndex));
}

UDlgNode* UDlgContext::GetMutableNodeFromIndex(int32 NodeIndex) const
{
	check(Dialogue);
	if (!Dialogue->IsValidNodeIndex(NodeIndex))
	{
		return nullptr;
	}

	return Dialogue->GetMutableNodeFromIndex(NodeIndex);
}

const UDlgNode* UDlgContext::GetNodeFromIndex(int32 NodeIndex) const
{
	check(Dialogue);
	if (!Dialogue->IsValidNodeIndex(NodeIndex))
	{
		return nullptr;
	}

	return Dialogue->GetMutableNodeFromIndex(NodeIndex);
}

UDlgNode* UDlgContext::GetMutableNodeFromGUID(const FGuid& NodeGUID) const
{
	check(Dialogue);
	if (!Dialogue->IsValidNodeGUID(NodeGUID))
	{
		return nullptr;
	}

	return Dialogue->GetMutableNodeFromGUID(NodeGUID);
}

const UDlgNode* UDlgContext::GetNodeFromGUID(const FGuid& NodeGUID) const
{
	check(Dialogue);
	if (!Dialogue->IsValidNodeGUID(NodeGUID))
	{
		return nullptr;
	}

	return Dialogue->GetMutableNodeFromGUID(NodeGUID);
}

bool UDlgContext::IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	check(Dialogue);
	if (const UDlgNode* Node = GetNodeFromIndex(NodeIndex))
	{
		return Node->CheckNodeEnterConditions(*this, AlreadyVisitedNodes);
	}

	return false;
}

bool UDlgContext::CanBeStarted(UDlgDialogue* InDialogue, const TMap<FGameplayTag, UObject*>& InParticipants)
{
	if (!ValidateParticipantsMapForDialogue(TEXT("CanBeStarted"), InDialogue, InParticipants, false))
	{
		return false;
	}

	// Get first participant
	UObject* FirstParticipant = nullptr;
	for (const auto& KeyValue : InParticipants)
	{
		if (KeyValue.Value)
		{
			FirstParticipant = KeyValue.Value;
			break;
		}
	}
	check(FirstParticipant != nullptr);

	// Create temporary context that is Garbage Collected after this function returns (hopefully)
	auto* Context = NewObject<UDlgContext>(FirstParticipant, UDlgContext::StaticClass());
	Context->Dialogue = InDialogue;
	Context->SetParticipants(InParticipants);

	// Evaluate edges/children of the start node
	for (const UDlgNode* StartNode : InDialogue->GetStartNodes())
	{
		for (const FDlgEdge& ChildLink : StartNode->GetNodeChildren())
		{
			if (ChildLink.Evaluate(*Context, {}))
			{
				// Simulate EnterNode
				UDlgNode* Node = Context->GetMutableNodeFromIndex(ChildLink.TargetIndex);
				if (Node && Node->HasAnySatisfiedChild(*Context, {}))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UDlgContext::StartWithContext(const FString& ContextString, UDlgDialogue* InDialogue, const TMap<FGameplayTag, UObject*>& InParticipants)
{
	const FString ContextMessage = ContextString.IsEmpty()
		? TEXT("Start")
		: FString::Printf(TEXT("%s - Start"), *ContextString);

	Dialogue = InDialogue;
	SetParticipants(InParticipants);
	if (!ValidateParticipantsMapForDialogue(ContextMessage, Dialogue, Participants))
	{
		return false;
	}

	// Evaluate edges/children of the start node

	for (const UDlgNode* StartNode : Dialogue->GetStartNodes())
	{
		for (const FDlgEdge& ChildLink : StartNode->GetNodeChildren())
		{
			if (ChildLink.Evaluate(*this, {}))
			{
				if (EnterNode(ChildLink.TargetIndex, true, {}))
				{
					return true;
				}
			}
		}
	}

	LogErrorWithContext(FString::Printf(
		TEXT("%s - FAILED because all possible start node condition failed. Edge conditions and children enter conditions from the start nodes are not satisfied"),
		*ContextMessage
	));
	return false;
}

bool UDlgContext::StartWithContextFromNode(
	const FString& ContextString,
	UDlgDialogue* InDialogue,
	const TMap<FGameplayTag, UObject*>& InParticipants,
	int32 StartNodeIndex,
	const FGuid& StartNodeGUID,
	const FDlgHistory& StartHistory,
	bool bEnterNode,
	bool bFireEnterEvents
)
{
	const FString ContextMessage = ContextString.IsEmpty()
		? TEXT("StartFromNode")
		: FString::Printf(TEXT("%s - StartFromNode"), *ContextString);

	Dialogue = InDialogue;
	SetParticipants(InParticipants);
	History = StartHistory;
	if (!ValidateParticipantsMapForDialogue(ContextMessage, Dialogue, Participants))
	{
		return false;
	}

	// Get the StartNodeIndex from the GUID
	if (StartNodeGUID.IsValid())
	{
		StartNodeIndex = GetNodeIndexForGUID(StartNodeGUID);
	}

	UDlgNode* Node = GetMutableNodeFromIndex(StartNodeIndex);
	if (!IsValid(Node))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("%s - FAILED because StartNodeIndex = %d  is INVALID. For StartNodeGUID = %s"),
			*ContextMessage, StartNodeIndex, *StartNodeGUID.ToString()
		));
		return false;
	}

	if (bEnterNode)
	{
		return EnterNode(StartNodeIndex, bFireEnterEvents, {});
	}

	ActiveNodeIndex = StartNodeIndex;
	SetNodeVisited(StartNodeIndex, Node->GetGUID());

	return Node->ReevaluateChildren(*this, {});
}

FString UDlgContext::GetContextString() const
{
	FString ContextParticipants;
	TSet<FString> ParticipantsNames;
	for (const auto& KeyValue : Participants)
	{
		ParticipantsNames.Add(KeyValue.Key.ToString());
	}

	return FString::Printf(
		TEXT("Dialogue = `%s`, ActiveNodeIndex = %d, Participants Names = `%s`"),
		Dialogue ? *Dialogue->GetPathName() : TEXT("INVALID"),
		ActiveNodeIndex,
		*FString::Join(ParticipantsNames, TEXT(", "))
	);
}

void UDlgContext::LogErrorWithContext(const FString& ErrorMessage) const
{
	FDlgLogger::Get().Error(GetErrorMessageWithContext(ErrorMessage));
}

FString UDlgContext::GetErrorMessageWithContext(const FString& ErrorMessage) const
{
	return FString::Printf(TEXT("%s.\nContext:\n\t%s"), *ErrorMessage, *GetContextString());
}

EDlgValidateStatus UDlgContext::IsValidParticipantForDialogue(const UDlgDialogue* Dialogue, const UObject* Participant)
{
	if (!IsValid(Participant))
	{
		return EDlgValidateStatus::ParticipantIsNull;
	}
	if (!IsValid(Dialogue))
	{
		return EDlgValidateStatus::DialogueIsNull;
	}

	// Does not implement interface
	if (!Participant->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
	{
		if (Participant->IsA<UBlueprint>())
		{
			return EDlgValidateStatus::ParticipantIsABlueprintClassAndDoesNotImplementInterface;
		}

		return EDlgValidateStatus::ParticipantDoesNotImplementInterface;
	}

	// We are more relaxed about this
	// Even if the user supplies more participants that required we still allow them to start the dialogue if the number of participants is bigger

	// Does the participant name exist in the Dialogue?
	// const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Participant);
	// if (!Dialogue->HasParticipant(ParticipantName))
	// {
	// 	return EDlgValidateStatus::DialogueDoesNotContainParticipant;
	// }

	return EDlgValidateStatus::Valid;
}

bool UDlgContext::ValidateParticipantForDialogue(
	const FString& ContextString,
	const UDlgDialogue* Dialogue,
	const UObject* Participant,
	bool bLog
)
{
	const EDlgValidateStatus Status = IsValidParticipantForDialogue(Dialogue, Participant);

	// Act as IsValidParticipantForDialogue
	if (!bLog)
	{
		return Status == EDlgValidateStatus::Valid;
	}

	switch (Status)
	{
		case EDlgValidateStatus::Valid:
			return true;

		case EDlgValidateStatus::DialogueIsNull:
			FDlgLogger::Get().Errorf(
				TEXT("%s - Dialogue is INVALID (not set or null).\nContext:\n\tParticipant = `%s`"),
				*ContextString, Participant ? *Participant->GetPathName() : TEXT("INVALID")
			);
			return false;

		case EDlgValidateStatus::ParticipantIsNull:
			FDlgLogger::Get().Errorf(
				TEXT("%s - Participant is INVALID (not set or null).\nContext:\n\tDialogue = `%s`"),
				*ContextString, Dialogue ? *Dialogue->GetPathName() : TEXT("INVALID")
			);
			return false;

		case EDlgValidateStatus::ParticipantDoesNotImplementInterface:
			FDlgLogger::Get().Errorf(
				TEXT("%s - Participant Path = `%s` does not implement the IDlgDialogueParticipant/UDlgDialogueParticipant interface.\nContext:\n\tDialogue = `%s`"),
				*ContextString, *Participant->GetPathName(), *Dialogue->GetPathName()
			);
			return false;

		case EDlgValidateStatus::ParticipantIsABlueprintClassAndDoesNotImplementInterface:
			FDlgLogger::Get().Errorf(
				TEXT("%s - Participant Path = `%s` is a Blueprint Class (from the content browser) and NOT a Blueprint Instance (from the level world).\nContext:\n\tDialogue = `%s`"),
				*ContextString, *Participant->GetPathName(), *Dialogue->GetPathName()
			);
			return false;

		// case EDlgValidateStatus::DialogueDoesNotContainParticipant:
	 //		FDlgLogger::Get().Errorf(
	 //			TEXT("%s - Participant Path = `%s` with ParticipantName = `%s` is NOT referenced (DOES) not exist inside the Dialogue.\nContext:\n\tDialogue = `%s`"),
	 //			*ContextString, *Participant->GetPathName(), *IDlgDialogueParticipant::Execute_GetParticipantName(Participant).ToString(), *Dialogue->GetPathName()
	 //		);
		// 	return false;

		default:
			FDlgLogger::Get().Errorf(TEXT("%s - ValidateParticipantForDialogue - Error EDlgValidateStatus Unhandled = %d"), *ContextString, static_cast<int32>(Status));
			return false;
	}
}

bool UDlgContext::ValidateParticipantsMapForDialogue(
	const FString& ContextString,
	const UDlgDialogue* Dialogue,
	const TMap<FGameplayTag, UObject*>& ParticipantsMap,
	bool bLog
)
{
	const FString ContextMessage = ContextString.IsEmpty()
		? FString::Printf(TEXT("ValidateParticipantsMapForDialogue"))
		: FString::Printf(TEXT("%s - ValidateParticipantsMapForDialogue"), *ContextString);

	if (!IsValid(Dialogue))
	{
		if (bLog)
		{
			FDlgLogger::Get().Errorf(TEXT("%s - FAILED because the supplied Dialogue Asset is INVALID (nullptr)"), *ContextMessage);
		}
		return false;
	}
	if (Dialogue->GetParticipantsData().Num() == 0)
	{
		if (bLog)
		{
			FDlgLogger::Get().Errorf(TEXT("%s - Dialogue = `%s` does not have any participants"), *ContextMessage, *Dialogue->GetPathName());
		}
		return false;
	}

	// Check if at least these participants are required
	const TMap<FGameplayTag, FDlgParticipantData>& DialogueParticipants = Dialogue->GetParticipantsData();
	TArray<FGameplayTag> ParticipantsRequiredArray;
	const int32 ParticipantsNum = DialogueParticipants.GetKeys(ParticipantsRequiredArray);
	TSet<FGameplayTag> ParticipantsRequiredSet{ParticipantsRequiredArray};

	// Iterate over Map
	for (const auto& KeyValue : ParticipantsMap)
	{
		const FGameplayTag ParticipantTag = KeyValue.Key;
		const UObject* Participant = KeyValue.Value;

		// We must check this otherwise we can't get the name
		if (!ValidateParticipantForDialogue(ContextMessage, Dialogue, Participant, bLog))
		{
			return false;
		}

		// Check the Map Key matches the Participant Tag
		// This should only happen if you constructed the map incorrectly by mistake
		// If you used ConvertArrayOfParticipantsToMap this should have NOT happened
		{
			const FGameplayTag ObjectParticipantTag = IDlgDialogueParticipant::Execute_GetParticipantTag(Participant);
			if (!ParticipantTag.MatchesTagExact(ObjectParticipantTag))
			{
				if (bLog)
				{
					FDlgLogger::Get().Errorf(
						TEXT("%s - The Map has a KEY Participant Tag = `%s` DIFFERENT to the VALUE of the Participant Path = `%s` with the Name = `%s` (KEY Participant Tag != VALUE Participant Tag)"),
						*ContextMessage, *ParticipantTag.ToString(), *Participant->GetPathName(), *ObjectParticipantTag.ToString()
					);
				}
				return false;
			}
		}

		// We found one participant from our set
		if (ParticipantsRequiredSet.Contains(ParticipantTag))
		{
			ParticipantsRequiredSet.Remove(ParticipantTag);
		}
		else
		{
			// Participant does note exist, just warn about it, we are relaxed about this
			if (bLog)
			{
				FDlgLogger::Get().Warningf(
					TEXT("%s - Participant Path = `%s` with Participant Tag = `%s` is NOT referenced (DOES) not exist inside the Dialogue. It is going to be IGNORED.\nContext:\n\tDialogue = `%s`"),
					*ContextMessage, *Participant->GetPathName(), *ParticipantTag.ToString(), *Dialogue->GetPathName()
				);
			}
		}
	}

	// Some participants are missing
	if (ParticipantsRequiredSet.Num() > 0)
	{
		if (bLog)
		{
			TArray<FString> ParticipantsMissing;
			for (const FName& Name : ParticipantsRequiredSet)
			{
				ParticipantsMissing.Add(Name.ToString());
			}

			const FString NameList = FString::Join(ParticipantsMissing, TEXT(", "));
			FDlgLogger::Get().Errorf(
				TEXT("%s - FAILED for Dialogue = `%s` because the following Participant Names are MISSING: `%s"),
				*ContextMessage,  *Dialogue->GetPathName(), *NameList
			);
		}
		return false;
	}

	return true;
}

bool UDlgContext::ConvertArrayOfParticipantsToMap(
	const FString& ContextString,
	const UDlgDialogue* Dialogue,
	const TArray<UObject*>& ParticipantsArray,
	TMap<FGameplayTag, UObject*>& OutParticipantsMap,
	bool bLog
)
{
	const FString ContextMessage = ContextString.IsEmpty()
		? FString::Printf(TEXT("ConvertArrayOfParticipantsToMap"))
		: FString::Printf(TEXT("%s - ConvertArrayOfParticipantsToMap"), *ContextString);

	// We don't allow to convert empty arrays
	OutParticipantsMap.Empty();
	if (ParticipantsArray.Num() == 0)
	{
		if (bLog)
		{
			FDlgLogger::Get().Errorf(
				TEXT("%s - Participants Array is EMPTY, can't convert anything. Dialogue = `%s`"),
				*ContextMessage, Dialogue ? *Dialogue->GetPathName() : TEXT("INVALID")
			);
		}
		return false;
	}

	for (int32 Index = 0; Index < ParticipantsArray.Num(); Index++)
	{
		UObject* Participant = ParticipantsArray[Index];
		const FString ContextMessageWithIndex = FString::Printf(TEXT("%s - Participant at Index = %d"), *ContextMessage,  Index);

		// We must check this otherwise we can't get the name
		if (!ValidateParticipantForDialogue(ContextMessageWithIndex, Dialogue, Participant, bLog))
		{
			return false;
		}

		// Is Duplicate?
		// Just warn the user about it, but still continue our conversion
		const FGameplayTag ParticipantTag = IDlgDialogueParticipant::Execute_GetParticipantTag(Participant);
		if (OutParticipantsMap.Contains(ParticipantTag))
		{
			if (bLog)
			{
				FDlgLogger::Get().Warningf(
					TEXT("%s - Participant Path = `%s`, Participant Tag = `%s` already exists in the Array. Ignoring it!"),
					*ContextMessageWithIndex, *Participant->GetPathName(), *ParticipantTag.ToString()
				);
			}
			continue;
		}

		OutParticipantsMap.Add(ParticipantTag, Participant);
	}

	return true;
}
