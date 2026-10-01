#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BGTypes.h"
#include "BGCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USkeletalMesh;
class UAnimInstance;

UCLASS(Blueprintable)
class BORROWEDGODS_API ABGCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ABGCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> PlaceholderBody;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> PlaceholderHead;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> PlaceholderBlade;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Costume;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Legs;
    float GaitTime = 0;
    void BuildPresentation();
    // Assign these on a Blueprint subclass when final character art arrives.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Art replacement") TObjectPtr<USkeletalMesh> CharacterAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Art replacement") TSubclassOf<UAnimInstance> AnimationClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Art replacement") FVector CharacterMeshOffset = FVector(0,0,-88);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Art replacement") FRotator CharacterMeshRotation = FRotator(0,-90,0);
    UPROPERTY(BlueprintReadOnly) float Health = 100;
    UPROPERTY(BlueprintReadOnly) float MaxHealth = 100;
    UPROPERTY(BlueprintReadOnly) float Stamina = 100;
    UPROPERTY(BlueprintReadOnly) float MaxStamina = 100;
    UPROPERTY(BlueprintReadOnly) float PowerCooldown = 0;
    float PowerCooldownMax = 7;
    float AttackCooldown = 0;
    float Invulnerable = 0;
    float DashRemaining = 0;
    float HitFlash = 0;
    float AttackAnim = 0;
    float ComboWindow = 0;
    float AttackWindup = 0;
    float RelicCooldown = 0;
    bool PendingFinisher = false;
    int32 ComboStep = 0;
    float GodCooldowns[3] = {0,0,0};
    FVector Facing = FVector(1,0,0);
    FVector MoveDirection = FVector::ZeroVector;
    FVector DashDirection = FVector::ZeroVector;
    void ResetForRun();
    void RefreshPower();
    bool Attack();
    void ResolveAttack();
    bool Dash();
    bool CastPower();
    void ReceiveHit(float Amount);
    void Heal(float Amount);
    UFUNCTION(BlueprintImplementableEvent, Category="Presentation") void OnAttackPresentation();
    UFUNCTION(BlueprintImplementableEvent, Category="Presentation") void OnPowerPresentation(EBGGod God);
    UFUNCTION(BlueprintImplementableEvent, Category="Presentation") void OnDamagePresentation(float Amount);
};
