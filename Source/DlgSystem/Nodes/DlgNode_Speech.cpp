// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_Speech.h"

#include "DlgSystem/DlgContext.h"
#include "DlgSystem/DlgConstants.h"
#include "DlgSystem/DlgHelper.h"
#include "DlgSystem/Logging/DlgLogger.h"
#include "DlgSystem/DlgLocalizationHelper.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR


void UDlgNode_Speech::OnCreatedInEditor()
{
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	if (NodeData != nullptr || Settings == nullptr || Settings->DefaultCustomNodeDataClass.IsNull())
	{
		return;
	}
	NodeData = NewObject<UDlgNodeData>(this, Settings->DefaultCustomNodeDataClass.Get(), NAME_None, GetMaskedFlags(RF_PropagateToSubObjects), NULL);
}


#if WITH_EDITOR
void UDlgNode_Speech::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const bool bTextChanged = PropertyName == GetMemberNameText();

	// rebuild text arguments
	if (bTextChanged || PropertyName == GetMemberNameTextArguments())
	{
		RebuildTextArguments(true);
	}
}

#endif

void UDlgNode_Speech::UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode)
{
	FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Text);
	Super::UpdateTextsValuesFromDefaultsAndRemappings(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_Speech::UpdateTextsNamespacesAndKeys(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode)
{
	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(GetOuter(), Settings, Text);
	Super::UpdateTextsNamespacesAndKeys(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_Speech::RebuildConstructedText(const UDlgContext& Context)
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


#if WITH_EDITOR

EDataValidationResult UDlgNode_Speech::IsDataValid(FDataValidationContext& Context) const
{
	bool b_valid = Super::IsDataValid(Context) != EDataValidationResult::Invalid;

	if (!GetNodeParticipantTag().IsValid())
	{
		b_valid = false;
		Context.AddError(FText::FromString(FString::Printf(TEXT("Speech node does not have a participant tag!"))));
	}

	return b_valid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

#endif // WITH_EDITOR


bool UDlgNode_Speech::HandleNodeEnter(UDlgContext& Context, bool bFireThisNodeEnterEvents, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	RebuildConstructedText(Context);
	const bool bResult = Super::HandleNodeEnter(Context, bFireThisNodeEnterEvents, NodesEnteredWithThisStep);

	// Handle virtual parent enter events for direct children
	if (bResult && bIsVirtualParent && Context.IsValidNodeIndex(VirtualParentFirstSatisfiedDirectChildIndex))
	{
		// Add to history
		Context.SetNodeVisited(
			VirtualParentFirstSatisfiedDirectChildIndex,
			Context.GetNodeGUIDForIndex(VirtualParentFirstSatisfiedDirectChildIndex)
		);

		// Fire all the direct child enter events
		if (bVirtualParentFireDirectChildEnterEvents)
		{
			if (UDlgNode* Node = Context.GetMutableNodeFromIndex(VirtualParentFirstSatisfiedDirectChildIndex))
			{
				Node->FireNodeEnterEvents(Context);
			}
		}
	}

	return bResult;
}

bool UDlgNode_Speech::ReevaluateChildren(UDlgContext& Context, TSet<const UDlgNode*> AlreadyEvaluated)
{
	if (bIsVirtualParent)
	{
		VirtualParentFirstSatisfiedDirectChildIndex = INDEX_NONE;
		Context.GetMutableOptionsArray().Empty();
		Context.GetAllMutableOptionsArray().Empty();

		// stop endless loop
		if (AlreadyEvaluated.Contains(this))
		{
			FDlgLogger::Get().Errorf(
				TEXT("ReevaluateChildren - Endless loop detected, a virtual parent became his own parent! "
					"This is not supposed to happen, the dialogue is terminated.\nContext:\n\t%s"),
				*Context.GetContextString()
			);
			return false;
		}

		AlreadyEvaluated.Add(this);

		for (const FDlgEdge& Edge : Children)
		{
			// Find first satisfied child
			if (Edge.Evaluate(Context, { this }))
			{
				if (UDlgNode* Node = Context.GetMutableNodeFromIndex(Edge.TargetIndex))
				{
					// Get Grandchildren
					const bool bResult = Node->ReevaluateChildren(Context, AlreadyEvaluated);
					if (bResult)
					{
						VirtualParentFirstSatisfiedDirectChildIndex = Edge.TargetIndex;
					}
					return bResult;
				}
			}
		}
		return false;
	}

	// Normal speech node
	return Super::ReevaluateChildren(Context, AlreadyEvaluated);
}


void UDlgNode_Speech::GetAssociatedParticipants(TArray<FGameplayTag>& OutArray) const
{
	Super::GetAssociatedParticipants(OutArray);
	for (const FDlgTextArgument& TextArgument : TextArguments)
	{
		if (UBSDlgFunctions::IsValidParticipantTag(TextArgument.ParticipantTag))
		{
			OutArray.AddUnique(TextArgument.ParticipantTag);
		}
	}
}
