#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BGEnemy.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UProceduralMeshComponent;
UCLASS(Blueprintable)
class BORROWEDGODS_API ABGEnemy : public AActor
{
    GENERATED_BODY()
public:
    ABGEnemy();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Head;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProceduralMeshComponent> Warning;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Costume;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Legs;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Shield;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Weapon;
    float GaitTime=0;
    UPROPERTY(BlueprintReadOnly) float Health = 55;
    UPROPERTY(BlueprintReadOnly) float MaxHealth = 55;
    UPROPERTY(BlueprintReadOnly) bool Boss = false;
    UPROPERTY(EditDefaultsOnly, Category="Art replacement") bool UsePrototypeCostume = true;
    bool Ranged = false;
    bool Armored = false;
    bool Dead = false;
    float AttackTimer = 1;
    float Windup = 0;
    float WindupMax = 1;
    float SlowTimer = 0;
    float WetTimer = 0;
    bool Enraged = false;
    int32 AttackPattern = 0;
    FString AttackName;
    float FlashTimer = 0;
    float StaggerTimer = 0;
    float ShieldOpen = 0;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> OwnMaterial;
    FVector AttackTarget;
    float AttackRadius = 170;
    void Configure(bool IsBoss, bool IsRanged, bool IsArmored = false);
    void Hit(float Amount, const FVector& Push = FVector::ZeroVector, bool Melee = false);
    void Interrupt(float Duration);
    UFUNCTION(BlueprintImplementableEvent, Category="Presentation") void OnHitPresentation(float Amount);
};
