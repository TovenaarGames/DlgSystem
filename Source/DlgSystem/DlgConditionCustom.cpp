// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

#include "DlgConditionCustom.h"

#include "DlgSystem/DlgHelper.h"

FString UDlgConditionCustom::GetEditorDisplayString_Implementation(UDlgDialogue* OwnerDialogue, const FGameplayTag& ParticipantTag)

{
	const FString TargetPreFix = UBSDlgFunctions::IsValidParticipantTag(ParticipantTag) ? FString::Printf(TEXT("[%s] "), *UBSDlgFunctions::GetParticipantLeafTagAsString(ParticipantTag)) : TEXT("");
#if WITH_EDITOR
	return TargetPreFix + GetClass()->GetDisplayNameText().ToString();
#else
	return TargetPreFix + GetName();
#endif
}