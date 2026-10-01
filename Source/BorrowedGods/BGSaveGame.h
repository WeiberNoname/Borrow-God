#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BGSaveGame.generated.h"

UCLASS()
class BORROWEDGODS_API UBGSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) int32 Version = 1;
    UPROPERTY(SaveGame) int32 Obols = 0;
    UPROPERTY(SaveGame) int32 HullLevel = 0;
    UPROPERTY(SaveGame) int32 BladeLevel = 0;
    UPROPERTY(SaveGame) int32 Expeditions = 0;
    UPROPERTY(SaveGame) int32 Victories = 0;
    UPROPERTY(SaveGame) int32 SelectedGod = 0;
    UPROPERTY(SaveGame) bool ReducedMotion = false;
    UPROPERTY(SaveGame) int32 HomewardProgress = 0;
    UPROPERTY(SaveGame) int32 LastDebtCount = 1;
    UPROPERTY(SaveGame) bool LastVoyageEscaped = false;
    UPROPERTY(SaveGame) TArray<FString> VoyageJournal;
    UPROPERTY(SaveGame) int32 SailorRecovery = 0;
    UPROPERTY(SaveGame) int32 MedicineDelivered = 0;
    UPROPERTY(SaveGame) int32 LastMedicine = 0;
    UPROPERTY(SaveGame) bool LastTribute = false;
    UPROPERTY(SaveGame) bool LastRiskyRoute = false;
    UPROPERTY(SaveGame) FString LastConsequence;
    UPROPERTY(SaveGame) bool Assistance = false;
    UPROPERTY(SaveGame) bool Muted = false;
    UPROPERTY(SaveGame) TMap<FName,FName> KeyBindings;
};
