// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"
#include "GameplayTagContainer.h"

#include "DlgSystem/DlgManager.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgDetailsPanelUtils.h"

class FDlgObject_CustomRowHelper;
class FDlgTextPropertyPickList_CustomRowHelper;
class FDlgMultiLineEditableTextBox_CustomRowHelper;
class FDlgIntTextBox_CustomRowHelper;
class UStringTable;
struct FAssetData;

/**
 * How the details customization panel looks for UDialogueGraphNode object
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class DLGSYSTEMEDITOR_API FDlgGraphNode_Details : public IDetailCustomization
{
	typedef FDlgGraphNode_Details Self;

public:
	// Makes a new instance of this detail layout class for a specific detail view requesting it
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<Self>(); }

	// IDetailCustomization interface
	/** Called when details should be customized */
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** Handler for when the text is changed */
	void HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);
	void HandleTextChanged(const FText& InText);

	/** Gets the ParticipantTags from all Dialogues. */
	TArray<FGameplayTag> GetDialoguesParticipantTags() const
	{
		return UDlgManager::GetDialoguesParticipantTags();
	}

	/** Gets the current Dialogue Participant Tags. */
	TArray<FGameplayTag> GetCurrentDialogueParticipantTags() const
	{
		return FDlgDetailsPanelUtils::GetDialogueSortedParticipantTags(Dialogue);
	}

	/** Gets the Speaker States from all Dialogues. */
	TArray<FName> GetDialoguesSpeakerStates() const
	{
		return UDlgManager::GetDialoguesSpeakerStates();
	}

	/** Handler for when text in the editable text box changed */
	void HandleParticipantTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
	{
		Dialogue->UpdateAndRefreshData();
	}

	/** Handler for when the speaker state is changed */
	void HandleSpeakerStateCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
	{
		Dialogue->UpdateAndRefreshData();
	}

	/** Parse a dialogue string table key into different components.*/
	bool ParseDialogueStringTableKey(const FString& InKey, FString& OutIdentifier, int32& OutSequenceNumber, FString& OutParticipantName) const;

	/** Compose a dialogue string table key from different components. */
	FString ComposeDialogueStringTableKey(const FString& Identifier, int32 SequenceNumber, const FString& ParticipantName) const;

	/** Create a speaker tag from the parsed participant name in a string table. */
	FGameplayTag CreateSpeakerTagFromName(const FString& ParticipantName) const;

	/** Calculate the Levenshstein distance between two words. This is used to determine string similarity. Sourced from https://github.com/guilhermeagostinelli/levenshtein/blob/master/levenshtein.cpp */
	int32 CalculateLevenshteinDistance(const FString& FromString, const FString& ToString) const;

	// The IsVirtualParent property changed
	void OnIsVirtualParentChanged();

	// Called when clicking the generate speech sequence button
	FReply OnGenerateSpeechSequenceButtonClicked();

private:
	/** Hold the reference to the Graph Node this represents */
	UDialogueGraphNode* GraphNode = nullptr;

	/** Cache some properties. */
	// Property Handles
	TSharedPtr<IPropertyHandle> IsVirtualParentPropertyHandle;
	TSharedPtr<IPropertyHandle> TextPropertyHandle;

	// Property rows
	IDetailPropertyRow* ParticipantTagPropertyRow;
	IDetailPropertyRow* ParticipantNamePropertyRow;
	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> SpeakerStatePropertyRow;
	TSharedPtr<FDlgMultiLineEditableTextBox_CustomRowHelper> TextPropertyRow;
	TSharedPtr<FDlgIntTextBox_CustomRowHelper> NodeIndexPropertyRow;
	IDetailPropertyRow* NodeDataPropertyRow = nullptr;
	TSharedPtr<FDlgObject_CustomRowHelper> NodeDataPropertyRow_CustomDisplay;
	IDetailPropertyRow* VoiceSoundWavePropertyRow = nullptr;
	IDetailPropertyRow* VoiceDialogueWavePropertyRow = nullptr;
	IDetailPropertyRow* GenericDataPropertyRow = nullptr;
	IDetailPropertyRow* ChildrenPropertyRow = nullptr;

	/** The details panel layout builder reference. */
	IDetailLayoutBuilder* DetailLayoutBuilder = nullptr;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;

	/** The string table from which a speech node sequence is generated. */
	UStringTable* StringTableSource;

	/** From what line we should parse our string table for speech node generation. */
	FText FromStringTableLineID;

	/** To what line we should parse our string table for speech node generation. */
	FText ToStringTableLineID;
};
