// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DlgTextArgumentCustom.h"
#include "GameplayTagContainer.h"

#include "DlgTextArgument.generated.h"

class IDlgDialogueParticipant;
class UDlgContext;


// Argument type, which defines both the type of the argument and the way the system will acquire the value
// NOTE: the values are out of order here for backwards compatibility
UENUM(BlueprintType)
enum class EDlgTextArgumentType : uint8
{
	// Calls GetParticipantDisplayName on the Participant
	DisplayName = 0	UMETA(DisplayName = "Participant Display Name", Hidden),

	// Calls GetParticipantGender on the Participant
	Gender			UMETA(DisplayName = "Participant Gender"),

	// Calls GetIntValue on the Participant
	DialogueInt 	UMETA(DisplayName = "Dialogue Int Value", Hidden),

	// Calls GetFloatValue on the Participant
	DialogueFloat	UMETA(DisplayName = "Dialogue Float Value", Hidden),

	// Gets the value from the Participant Int Variable
	ClassInt 		UMETA(DisplayName = "Class Int Variable", Hidden),

	// Gets the value from the Participant Float Variable
	ClassFloat		UMETA(DisplayName = "Class Float Variable", Hidden),

	// Gets the value from the Participant Text Variable
	ClassText		UMETA(DisplayName = "Class Text Variable", Hidden),


	// User Defined Text Argument, calls GetText on the custom text argument object.
	//
	// 1. Create a new Blueprint derived from DlgTextArgumentCustom (or DlgTextArgumentCustomHideCategories)
	// 2. Override GetText
	Custom						UMETA(DisplayName = "Custom Text Argument")
};

/**
 * An argument is a variable to extend node texts with dynamic values runtime
 * It can be inserted to the FText, the same way FText::Format works
 * See: https://docs.unrealengine.com/en-us/Gameplay/Localization/Formatting
 */
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgTextArgument
{
	GENERATED_USTRUCT_BODY()

public:
	//
	// ICppStructOps Interface
	//

	bool operator==(const FDlgTextArgument& Other) const
	{
		return	DisplayString == Other.DisplayString &&
			Type == Other.Type &&
			ParticipantTag.MatchesTagExact(Other.ParticipantTag) &&
			VariableName == Other.VariableName &&
			CustomTextArgument == Other.CustomTextArgument;
	}

	//
	// Own methods
	//

	// Construct the argument for usage in FText::Format
	FFormatArgumentValue ConstructFormatArgumentValue(const UDlgContext& Context, const FGameplayTag& NodeOwner) const;

	// Helper method to update the array InOutArgumentArray with the new arguments from Text.
	static void UpdateTextArgumentArray(const FText& Text, TArray<FDlgTextArgument>& InOutArgumentArray);

	static FString ArgumentTypeToString(EDlgTextArgumentType Type);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue|TextArgument")
	FString DisplayString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|TextArgument")
	EDlgTextArgumentType Type = EDlgTextArgumentType::Custom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue|TextArgument", meta=(DeprecatedProperty, DeprecatedMessage="Use ParticipantTag"))
	FName ParticipantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|TextArgument")
	FGameplayTag ParticipantTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|TextArgument")
	FName VariableName;

	// User Defined Text Argument, calls GetText on the custom text argument object.
	//
	// 1. Create a new Blueprint derived from DlgTextArgumentCustom (or DlgTextArgumentCustomHideCategories)
	// 2. Override GetText
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Dialogue|TextArgument")
	UDlgTextArgumentCustom* CustomTextArgument = nullptr;
};

template<>
struct TStructOpsTypeTraits<FDlgTextArgument> : public TStructOpsTypeTraitsBase2<FDlgTextArgument>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
};
