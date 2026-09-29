// Fill out your copyright notice in the Description page of Project Settings.


#include "Utilities/InputFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerInput.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "EnhancedInputSubsystems.h"

void UInputFunctionLibrary::SimulateKeyUse(const UObject* WorldContextObject, FKey Key, float Delta, int32 PlayerIndex)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex);
    if (!PC || !PC->PlayerInput)
    {
        UKismetSystemLibrary::PrintString(WorldContextObject, FString::Printf(TEXT("SimulateKeyUse: no PlayerController/PlayerInput")));
        return;
    }

    FInputKeyEventArgs Args = FInputKeyEventArgs::CreateSimulated(Key, EInputEvent::IE_Axis, Delta, 1,
        IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetLocalPlayer()->GetPlatformUserId()));

    PC->PlayerInput->InputKey(Args);
}

void UInputFunctionLibrary::InjectInputAction(const UObject* WorldContextObject, UInputAction* Action, float Value, int32 PlayerIndex)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex);
    if (!PC || !Action) {
        UKismetSystemLibrary::PrintString(WorldContextObject, FString::Printf(TEXT("InjectInputAction: no PlayerController/Action")));
        return;
    }

    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        if (UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput())
        {
            PlayerInput->InjectInputForAction(Action, FInputActionValue(Value));
            return;
        }
    }

    UKismetSystemLibrary::PrintString(WorldContextObject, FString::Printf(TEXT("InjectInputAction: no LocalPlayerSubsystem/PlayerInput")));
}
