#include "BGCharacter.h"
#include "BGGameMode.h"
#include "BGEnemy.h"
#include "BGSaveGame.h"
#include "BGVisuals.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABGCharacter::ABGCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(34, 88);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 520;
    GetCharacterMovement()->BrakingDecelerationWalking = 3200;
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 3100;
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->SetRelativeRotation(FRotator(-56,-45,0));
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 7;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom);
    Camera->FieldOfView = 64;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Capsule(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
    PlaceholderBody->SetupAttachment(RootComponent);
    PlaceholderBody->SetStaticMesh(Capsule.Object);
    PlaceholderBody->SetRelativeScale3D(FVector(.55,.36,.68));
    PlaceholderBody->SetRelativeLocation(FVector(0,0,4));
    PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PlaceholderHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderHead"));
    PlaceholderHead->SetupAttachment(RootComponent);
    PlaceholderHead->SetStaticMesh(Sphere.Object);
    PlaceholderHead->SetRelativeScale3D(FVector(.37,.32,.39));
    PlaceholderHead->SetRelativeLocation(FVector(0,0,61));
    PlaceholderHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PlaceholderBlade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBlade"));
    PlaceholderBlade->SetupAttachment(RootComponent);
    PlaceholderBlade->SetStaticMesh(Cube.Object);
    PlaceholderBlade->SetRelativeScale3D(FVector(1.25,.065,.075));
    PlaceholderBlade->SetRelativeLocation(FVector(65,39,0));
    PlaceholderBlade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABGCharacter::BuildPresentation()
{
    if(!Costume.IsEmpty() || CharacterAsset) return;
    auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    auto* Cloth=G->Material(FLinearColor(.035,.095,.115));
    auto* Bronze=G->GoldMaterial.Get();
    auto* Dark=G->Material(FLinearColor(.015,.027,.032));
    auto* Skin=G->Material(FLinearColor(.40,.25,.15));
    auto* Glow=G->Material(FLinearColor(.15,.85,.78),2);
    auto Add=[&](UStaticMesh* Geometry,FVector P,FVector S,UMaterialInterface* M,FRotator R=FRotator::ZeroRotator)
    { auto* C=BGVisuals::Part(this,RootComponent,Geometry,P,S,M,R); Costume.Add(C); return C; };
    // A cloaked bronze captain: legs, pauldrons, helmet crest, shield and sword.
    Add(G->CubeMesh,FVector(-23,0,-5),FVector(.08,.54,.95),Cloth,FRotator(0,0,-12));
    Add(G->CylinderMesh,FVector(0,0,-27),FVector(.58,.4,.10),Bronze);
    for(int32 Sign : {-1,1})
    {
        Legs.Add(Add(G->CubeMesh,FVector(0,Sign*13,-53),FVector(.18,.18,.53),Cloth));
        Add(G->CubeMesh,FVector(9,Sign*13,-78),FVector(.35,.21,.15),Dark);
        Add(G->SphereMesh,FVector(0,Sign*29,26),FVector(.32,.30,.23),Bronze);
        Add(G->CylinderMesh,FVector(4,Sign*30,1),FVector(.16,.16,.5),Skin,FRotator(0,0,Sign*14));
        Add(G->CubeMesh,FVector(6,Sign*30,-18),FVector(.22,.23,.22),Bronze);
    }
    Add(G->CubeMesh,FVector(16,0,62),FVector(.11,.41,.085),Dark);
    Add(G->CubeMesh,FVector(-3,0,84),FVector(.45,.095,.25),Cloth);
    Add(G->CubeMesh,FVector(5,0,13),FVector(.42,.075,.61),Bronze,FRotator(0,0,-28));
    Add(G->SphereMesh,FVector(27,-28,25),FVector(.11),Glow);
    Add(G->CylinderMesh,FVector(5,-43,-2),FVector(.59,.59,.075),Bronze,FRotator(90,0,0));
    Add(G->SphereMesh,FVector(9,-45,-2),FVector(.25,.07,.25),G->PlayerMaterial);
}

void ABGCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (CharacterAsset)
    {
        GetMesh()->SetSkeletalMesh(CharacterAsset);
        GetMesh()->SetRelativeLocation(CharacterMeshOffset);
        GetMesh()->SetRelativeRotation(CharacterMeshRotation);
        if (AnimationClass) GetMesh()->SetAnimInstanceClass(AnimationClass);
        PlaceholderBody->SetVisibility(false);
        PlaceholderHead->SetVisibility(false);
        PlaceholderBlade->SetVisibility(false);
    }
}

void ABGCharacter::ResetForRun()
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    MaxHealth = BGRules::MaxHealth(G->Save->HullLevel);
    MaxStamina = BGRules::MaxStamina(G->God);
    Health = MaxHealth; Stamina = MaxStamina;
    AttackCooldown = PowerCooldown = Invulnerable = DashRemaining = 0;
    ComboStep=0; ComboWindow=0;
    AttackWindup=RelicCooldown=0; PendingFinisher=false; AttackAnim=HitFlash=0;
    for(float& Cooldown:GodCooldowns) Cooldown=0;
    SetActorLocation(FVector(-450,-450,100));
    GetCharacterMovement()->StopMovementImmediately();
    PlaceholderBody->SetMaterial(0, G->PlayerMaterial);
    PlaceholderHead->SetMaterial(0, G->GoldMaterial);
    PlaceholderBlade->SetMaterial(0, G->GoldMaterial);
    BuildPresentation();
    for(auto& Part:Costume) Part->SetVisibility(!CharacterAsset);
    CameraBoom->bEnableCameraLag = !G->Save->ReducedMotion;
}

void ABGCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying()) { GetCharacterMovement()->StopMovementImmediately(); return; }
    RelicCooldown=FMath::Max(0.f,RelicCooldown-Dt);
    if(AttackWindup>0) { AttackWindup-=Dt; if(AttackWindup<=0) ResolveAttack(); }
    AttackCooldown = FMath::Max(0.f, AttackCooldown-Dt);
    for(float& Cooldown:GodCooldowns) Cooldown=FMath::Max(0.f,Cooldown-Dt);
    RefreshPower();
    ComboWindow=FMath::Max(0.f,ComboWindow-Dt);
    Invulnerable = FMath::Max(0.f, Invulnerable-Dt);
    Stamina = FMath::Min(MaxStamina, Stamina + 24*Dt);
    if (DashRemaining > 0)
    {
        DashRemaining -= Dt;
        AddActorWorldOffset(DashDirection * 1500 * Dt, true);
    }
    else AddMovementInput(MoveDirection*(AttackWindup>0 ? .55f : 1.f));
    SetActorRotation(Facing.Rotation());
    GaitTime+=Dt*(MoveDirection.IsNearlyZero() ? 0 : 10);
    for(int32 I=0;I<Legs.Num();++I) Legs[I]->SetRelativeRotation(FRotator(MoveDirection.IsNearlyZero() ? 0 : FMath::Sin(GaitTime+I*PI)*19,0,0));
    AttackAnim = FMath::Max(0.f, AttackAnim-Dt);
    PlaceholderBlade->SetRelativeRotation(FRotator(0, AttackWindup>0 ? -85 : AttackAnim > 0 ? -65 + (1-AttackAnim/.24f)*150 : 0, 0));
    HitFlash = FMath::Max(0.f, HitFlash-Dt);
    PlaceholderBody->SetVisibility(!CharacterAsset && (HitFlash <= 0 || FMath::Fmod(HitFlash,.1f) < .06f));
    FVector P = GetActorLocation();
    P.X = FMath::Clamp(P.X,-1550.f,1550.f); P.Y = FMath::Clamp(P.Y,-1550.f,1550.f);
    if (P.Z < -100) P.Z = 100;
    SetActorLocation(P, false);
}

bool ABGCharacter::Attack()
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying() || AttackCooldown > 0 || DashRemaining > 0) return false;
    if(ComboWindow<=0) ComboStep=0;
    PendingFinisher=ComboStep==2;
    AttackCooldown = PendingFinisher ? .56f : .34f;
    AttackWindup=PendingFinisher ? .14f : .08f;
    ComboStep=(ComboStep+1)%3; ComboWindow=1.1f;
    OnAttackPresentation();
    G->Feedback(0);
    return true;
}

void ABGCharacter::ResolveAttack()
{
    auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    if(!G || G->Phase!=EBGPhase::Combat) return;
    AttackAnim=.24f;
    const bool Finisher=PendingFinisher;
    int32 Hits=0;
    G->Burst(GetActorLocation()+Facing*125,Finisher ? 210 : 160,FLinearColor(.9f,.78f,.48f),.18f);
    TArray<TObjectPtr<ABGEnemy>> Targets = G->Enemies;
    for (auto& E : Targets)
        if (IsValid(E) && !E->Dead && BGRules::InMeleeArc(Facing, E->GetActorLocation()-GetActorLocation(), 245))
        {
            if(Finisher)
            {
                E->SlowTimer=FMath::Max(E->SlowTimer,.7f); E->ShieldOpen=1.2f;
                if(G->Relic==EBGRelic::Oathstone) E->Interrupt(.85f);
                G->FloatText(E->GetActorLocation()+FVector(0,0,200),TEXT("OATHBREAK"),FLinearColor(.95,.8,.4));
            }
            E->Hit((24 + G->Save->BladeLevel*4 + G->RunDamageBonus)*(Finisher ? 1.6f : 1.f), Facing*(Finisher ? 120 : 35),true);
            ++Hits;
        }
    if(Hits>0)
    {
        G->Impact(Finisher);
        if(Finisher && G->Relic==EBGRelic::Oathstone) { Stamina=FMath::Min(MaxStamina,Stamina+24); G->FloatText(GetActorLocation()+FVector(0,0,130),TEXT("OATHSTONE / +24 STAMINA"),FLinearColor(.3,.9,.7)); }
    }
}

bool ABGCharacter::Dash()
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying() || Stamina < 30 || DashRemaining > 0) return false;
    AttackWindup=0;
    if(G->Relic==EBGRelic::FerrymanThread && RelicCooldown<=0)
    {
        for(auto& E:G->Enemies) if(IsValid(E) && E->Windup>0 && E->Windup<=.35f && FVector::Dist2D(GetActorLocation(),E->AttackTarget)<E->AttackRadius+20)
        {
            Heal(8); G->Wrath=FMath::Max(0.f,G->Wrath-12); RelicCooldown=1.5f;
            G->FloatText(GetActorLocation()+FVector(0,0,185),TEXT("THREAD / PERFECT DODGE"),FLinearColor(.3,.9,.7)); break;
        }
    }
    Stamina -= 30;
    DashRemaining = .20f;
    Invulnerable = .28f;
    DashDirection = MoveDirection.IsNearlyZero() ? Facing : MoveDirection.GetSafeNormal();
    G->Burst(GetActorLocation(),110,BGRules::GodColor(G->God),.25f);
    G->Feedback(3);
    return true;
}

bool ABGCharacter::CastPower()
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying() || PowerCooldown > 0) return false;
    const EBGGod God = G->God;
    PowerCooldownMax = (God == EBGGod::Poseidon ? 6.f : God == EBGGod::Zeus ? 7.f : 8.f)*G->RunCooldownMultiplier;
    PowerCooldown = PowerCooldownMax;
    GodCooldowns[static_cast<int32>(God)]=PowerCooldown;
    const float Radius = (God == EBGGod::Zeus ? 740 : God == EBGGod::Poseidon ? 580 : 440)+(God==EBGGod::Poseidon && G->TributeAccepted ? 100 : 0);
    const float Damage = (God == EBGGod::Zeus ? 62.f : God == EBGGod::Poseidon ? 42.f : 38.f)+G->RunDamageBonus+(G->TributeAccepted ? 20 : 0);
    int Hits = 0;
    G->Burst(GetActorLocation(),Radius,BGRules::GodColor(God),.65f);
    OnPowerPresentation(God);
    G->Feedback(6);
    TArray<TObjectPtr<ABGEnemy>> Targets = G->Enemies;
    for (auto& E : Targets)
    {
        if (!IsValid(E) || E->Dead) continue;
        const FVector Delta = E->GetActorLocation()-GetActorLocation();
        if (Delta.Size2D() <= Radius)
        {
            float HitDamage=Damage;
            if(God==EBGGod::Poseidon) { E->SlowTimer=3; E->WetTimer=5; }
            if(God==EBGGod::Zeus && E->WetTimer>0)
            {
                HitDamage+=26; E->WetTimer=0;
                G->FloatText(E->GetActorLocation()+FVector(0,0,200),TEXT("CONDUCTION +26"),BGRules::GodColor(God));
                G->Burst(E->GetActorLocation(),130,BGRules::GodColor(God),.28f);
            }
            const FVector HitPosition=E->GetActorLocation();
            const bool Conducted=God==EBGGod::Zeus && HitDamage>Damage;
            E->Hit(HitDamage, God == EBGGod::Poseidon ? Delta.GetSafeNormal2D()*210 : FVector::ZeroVector);
            if(Conducted && G->Relic==EBGRelic::Stormglass)
            {
                G->Burst(HitPosition,320,FLinearColor(.85,.73,1),.3);
                for(auto& Other:Targets) if(IsValid(Other) && Other!=E && !Other->Dead && FVector::Dist2D(Other->GetActorLocation(),HitPosition)<=320) Other->Hit(18);
                G->FloatText(HitPosition+FVector(0,0,230),TEXT("STORMGLASS / ARC"),FLinearColor(.85,.73,1));
            }
            ++Hits;
        }
    }
    if (God == EBGGod::Hades && Hits > 0) Heal(FMath::Min(30,Hits*12));
    if (G->HasGod(EBGGod::Zeus))
    {
        Health = FMath::Max(1.f,Health-8);
        G->FloatText(GetActorLocation()+FVector(0,0,120),TEXT("-8  BLOOD PRICE"),FLinearColor(1,.4f,.3f));
    }
    G->Wrath = FMath::Min(100.f,G->Wrath+(God == EBGGod::Poseidon ? 23 : 15));
    return true;
}

void ABGCharacter::RefreshPower()
{
    auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    if(!G) return;
    PowerCooldown=GodCooldowns[static_cast<int32>(G->God)];
    PowerCooldownMax=(G->God==EBGGod::Poseidon ? 6.f : G->God==EBGGod::Zeus ? 7.f : 8.f)*G->RunCooldownMultiplier;
}

void ABGCharacter::ReceiveHit(float Amount)
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying() || Invulnerable > 0) return;
    Amount*=G->Save->Assistance ? .65f : 1.f;
    Health = FMath::Max(0.f, Health-Amount);
    Invulnerable = .55f;
    HitFlash = .35f;
    G->DamageFeedback=.24f; G->Feedback(4);
    OnDamagePresentation(Amount);
    G->FloatText(GetActorLocation()+FVector(0,0,130),FString::Printf(TEXT("-%d"),FMath::RoundToInt(Amount)),FLinearColor(1,.35f,.25f));
    if (Health <= 0) G->FinishExpedition(false);
}

void ABGCharacter::Heal(float Amount)
{
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    const float Before = Health;
    Health = FMath::Min(MaxHealth,Health+Amount);
    if (G && Health > Before) G->FloatText(GetActorLocation()+FVector(0,0,130),FString::Printf(TEXT("+%d"),FMath::RoundToInt(Health-Before)),FLinearColor(.3f,1,.7f));
}
