#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGTypes.h"
#include "BGGameMode.generated.h"

class ABGCharacter;
class ABGEnemy;
class UBGSaveGame;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class AStaticMeshActor;
class UAudioComponent;
class USoundWaveProcedural;
class UTextureCube;

struct FBGFloatText
{
    FVector Position;
    FString Text;
    FLinearColor Color;
    float Life = 1.2f;
};
struct FBGEffect
{
    TWeakObjectPtr<AActor> Actor;
    float Age = 0;
    float Duration = 0.4f;
    float Radius = 100;
};

UCLASS()
class BORROWEDGODS_API ABGGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABGGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(BlueprintReadOnly) EBGPhase Phase = EBGPhase::Harbor;
    UPROPERTY(BlueprintReadOnly) EBGGod God = EBGGod::Poseidon;
    UPROPERTY(BlueprintReadOnly) int32 BorrowedGods = 1;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<UBGSaveGame> Save;
    UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<ABGEnemy>> Enemies;
    UPROPERTY(EditDefaultsOnly, Category="Art replacement") TSubclassOf<ABGEnemy> EnemyClass;
    UPROPERTY(EditDefaultsOnly, Category="Art replacement") TSubclassOf<ABGEnemy> GuardianClass;
    UPROPERTY(BlueprintReadOnly) int32 Encounter = 0;
    UPROPERTY(BlueprintReadOnly) int32 CarriedObols = 0;
    int32 Kills = 0;
    int32 LastReward = 0;
    int32 RunDamageBonus = 0;
    int32 RunBoonCount = 0;
    float RunCooldownMultiplier = 1;
    float Wrath = 0;
    float RunTime = 0;
    float EncounterDelay = 0;
    float HazardTimer = 0;
    FVector HazardPosition;
    bool Paused = false;
    bool JournalOpen = false;
    bool HelpOpen = false;
    bool CrewOpen = false;
    bool SettingsOpen = false;
    int32 CrewSpeaker = 0;
    bool AwaitingExtraction = false;
    bool SmokeTest = false;
    bool VisualPreview = false;
    bool SaveSucceeded = true;
    bool RiskyRoute = false;
    bool TributeAccepted = false;
    bool BargainResolved = false;
    bool SeaStrike = false;
    int32 Medicine = 0;
    int32 SeaStrikesRemaining = 0;
    float SeaStrikeDelay = 0;
    float HitStop = 0;
    float ImpactFeedback = 0;
    float DamageFeedback = 0;
    EBGRelic Relic = EBGRelic::None;
    FString VoyageConsequence;
    UPROPERTY() TArray<TObjectPtr<UAudioComponent>> AudioVoices;
    UPROPERTY() TArray<TObjectPtr<USoundWaveProcedural>> AudioWaves;
    float AudioTimers[4] = {0,0,0,0};
    int32 AudioVoice = 0;
    FString Toast;
    float ToastLife = 0;
    TArray<FBGFloatText> FloatingText;
    TArray<FBGEffect> Effects;
    UPROPERTY() TObjectPtr<UMaterialInterface> BaseMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> WaterMaterial;
    UPROPERTY() TObjectPtr<UTextureCube> AmbientCube;
    UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
    UPROPERTY() TObjectPtr<UStaticMesh> SphereMesh;
    UPROPERTY() TObjectPtr<UStaticMesh> CylinderMesh;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> StoneMaterial;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> GoldMaterial;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> PlayerMaterial;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> EnemyMaterial;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> WarningMaterial;
    UPROPERTY() TObjectPtr<AActor> HazardDisc;
    UPROPERTY() TObjectPtr<AActor> ExitMarker;
    ABGCharacter* Player() const;
    bool IsPlaying() const;
    void BuildIsland();
    void StartExpedition();
    void Depart();
    void ChooseRoute(int32 Index);
    void ChooseRelic(int32 Index);
    void ChooseBargain(int32 Index);
    void StartHazard(bool FromSea);
    bool HasActiveVoyage() const;
    void Feedback(int32 Cue);
    void Impact(bool Finisher);
    void SpawnEncounter();
    void EnemyKilled(ABGEnemy* Enemy);
    void ChooseOffering(int32 Index);
    TArray<int32> OfferingOptions() const;
    bool HasGod(EBGGod Value) const;
    int32 DebtCount() const;
    void SwitchGod(int32 Index);
    void CycleGod();
    FString BurdenSummary() const;
    FString CrewDialogue() const;
    void FinishExpedition(bool Success);
    void ReturnToHarbor();
    bool Upgrade(bool Blade);
    void SelectGod(int32 Index);
    void SaveProgress();
    void Interact();
    void Notify(const FString& Message);
    void FloatText(const FVector& At, const FString& Text, FLinearColor Color);
    void Burst(const FVector& At, float Radius, FLinearColor Color, float Duration = 0.4f);
    UMaterialInstanceDynamic* Material(FLinearColor Color, float Glow = 0, float Roughness = .82f, float Metallic = 0);
    AStaticMeshActor* Shape(UStaticMesh* Mesh, FVector Position, FVector Scale, UMaterialInterface* Mat, bool Collision = false, FRotator Rotation = FRotator::ZeroRotator);
    void HandleAction(FName Action);
    FString Objective() const;
    FString SaveSlot() const;
    void TickSmokeTest(float Dt);
    void TickVisualPreview(float Dt);
    int32 SmokeStep = 0;
    float SmokeTime = 0;
    bool SmokeFailed = false;
    TSet<FName> SmokeScreens;
    FVector SmokeMoveStart;
    void Check(bool Condition, const FString& Message);
};
