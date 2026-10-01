#include "BGEnemy.h"
#include "BGGameMode.h"
#include "BGCharacter.h"
#include "BGSaveGame.h"
#include "BGVisuals.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABGEnemy::ABGEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    Body->SetupAttachment(RootComponent); Body->SetStaticMesh(Cylinder.Object);
    Body->SetRelativeScale3D(FVector(.62,.62,1.1)); Body->SetRelativeLocation(FVector(0,0,55));
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
    Head->SetupAttachment(RootComponent); Head->SetStaticMesh(Sphere.Object);
    Head->SetRelativeScale3D(FVector(.48)); Head->SetRelativeLocation(FVector(0,0,133));
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Warning = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("AttackTelegraph"));
    Warning->SetupAttachment(RootComponent);
    Warning->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Warning->SetVisibility(false); Warning->SetCastShadow(false);
}

void ABGEnemy::BeginPlay()
{
    Super::BeginPlay();
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    BGVisuals::RingMesh(Warning,50,2.5);
    Body->SetMaterial(0,G->EnemyMaterial);
    Head->SetMaterial(0,G->GoldMaterial);
    Warning->SetMaterial(0,G->WarningMaterial);
}

void ABGEnemy::Configure(bool IsBoss,bool IsRanged,bool IsArmored)
{
    Boss = IsBoss; Ranged = IsRanged; Armored=IsArmored && !Boss;
    MaxHealth = Health = Boss ? 480 : Armored ? 120 : Ranged ? 60 : 78;
    AttackRadius = Boss ? 320 : Ranged ? 150 : 155;
    Body->SetRelativeScale3D(Boss ? FVector(1.4,.95,1.3) : FVector(.62,.40,.72));
    Body->SetRelativeLocation(FVector(0,0,Boss ? 210 : 88));
    Head->SetRelativeScale3D(Boss ? FVector(.80,.66,.84) : FVector(.40,.34,.42));
    Head->SetRelativeLocation(FVector(0,0,Boss ? 317 : 149));
    OwnMaterial=GetWorld()->GetAuthGameMode<ABGGameMode>()->Material(Boss ? FLinearColor(.48,.18,.14) : Armored ? FLinearColor(.45,.38,.18) : Ranged ? FLinearColor(.39,.28,.50) : FLinearColor(.48,.18,.14));
    Body->SetMaterial(0,OwnMaterial);
    AttackTimer = 1.2f + FMath::FRandRange(0,.6);
    if(!UsePrototypeCostume) return;
    auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    auto* Bronze=G->GoldMaterial.Get();
    auto* Dark=G->Material(FLinearColor(.025,.04,.055));
    auto* Cloth=G->Material(Ranged ? FLinearColor(.19,.08,.26) : FLinearColor(.15,.055,.038));
    auto* Eyes=G->Material(Boss ? FLinearColor(1,.19,.04) : Ranged ? FLinearColor(.65,.27,1) : FLinearColor(1,.45,.15),3);
    const float K=Boss ? 2.f : 1.f;
    auto Add=[&](UStaticMesh* Mesh,FVector P,FVector S,UMaterialInterface* M,FRotator R=FRotator::ZeroRotator)
    { auto* C=BGVisuals::Part(this,RootComponent,Mesh,P*K,S*K,M,R); Costume.Add(C); return C; };
    for(int32 Sign : {-1,1})
    {
        Legs.Add(Add(G->CubeMesh,FVector(0,Sign*15,32),FVector(.22,.24,.57),Boss ? Bronze : Dark));
        Add(G->CubeMesh,FVector(9,Sign*15,7),FVector(.40,.28,.14),Dark);
        Add(G->SphereMesh,FVector(0,Sign*35,114),FVector(.42,.37,.28),Bronze);
        Add(G->CylinderMesh,FVector(5,Sign*39,85),FVector(.22,.22,.57),OwnMaterial);
        Add(G->CubeMesh,FVector(7,Sign*39,62),FVector(.28,.28,.28),Bronze);
    }
    Add(G->CylinderMesh,FVector(0,0,58),FVector(.70,.46,.12),Bronze);
    Add(G->CubeMesh,FVector(18,0,Boss ? 161 : 151),FVector(.12,.41,.08),Dark);
    Add(G->CubeMesh,FVector(23,0,Boss ? 164 : 154),FVector(.03,.26,.045),Eyes);
    Add(G->CubeMesh,FVector(-30,0,84),FVector(.10,.61,Ranged ? 1.45f : .8f),Cloth,FRotator(0,0,-9));
    Add(G->CubeMesh,FVector(-4,0,Boss ? 185 : 174),FVector(.43,.10,.23),Cloth);
    if(Armored || Boss)
    {
        Shield=Add(G->CylinderMesh,FVector(25,-48,84),FVector(Boss ? .85f : 1.08f,Boss ? .85f : 1.08f,.09),Bronze,FRotator(90,0,0));
        Add(G->SphereMesh,FVector(32,-48,84),FVector(.12,.33,.33),OwnMaterial);
    }
    Weapon=Add(G->CubeMesh,FVector(37,42,76),FVector(Ranged ? .10f : 1.10f,.10,Ranged ? 1.95f : .09f),Bronze);
    if(Ranged) Add(G->SphereMesh,FVector(37,42,177),FVector(.27),Eyes);
    if(Boss)
    {
        Add(G->CubeMesh,FVector(88,42,76),FVector(.45,.45,.65),Bronze);
        Add(G->CubeMesh,FVector(51,0,108),FVector(.065,.20,.30),Eyes);
        for(int32 Sign : {-1,1}) Add(G->CubeMesh,FVector(0,Sign*24,181),FVector(.12,.12,.5),Bronze,FRotator(0,0,Sign*22));
    }
}

void ABGEnemy::Tick(float Dt)
{
    Super::Tick(Dt);
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if (!G || !G->IsPlaying() || Dead) return;
    ABGCharacter* P = G->Player();
    if (!P) return;
    SlowTimer = FMath::Max(0.f,SlowTimer-Dt);
    WetTimer = FMath::Max(0.f,WetTimer-Dt);
    FlashTimer = FMath::Max(0.f,FlashTimer-Dt);
    ShieldOpen=FMath::Max(0.f,ShieldOpen-Dt);
    if(OwnMaterial) OwnMaterial->SetVectorParameterValue(TEXT("Tint"),FlashTimer>0 ? FLinearColor(1,.85,.65) : Boss && Enraged ? FLinearColor(.65,.22,.06) : Armored ? FLinearColor(.45,.38,.18) : Ranged ? FLinearColor(.39,.28,.50) : FLinearColor(.48,.18,.14));
    if(StaggerTimer>0) { StaggerTimer=FMath::Max(0.f,StaggerTimer-Dt); return; }
    const FVector Delta = P->GetActorLocation()-GetActorLocation();
    SetActorRotation(Delta.Rotation());
    GaitTime+=Dt*7;
    for(int32 I=0;I<Legs.Num();++I) Legs[I]->SetRelativeRotation(FRotator(Windup>0 || Delta.Size2D()<125 ? 0 : FMath::Sin(GaitTime+I*PI)*17,0,0));
    if(Shield) Shield->SetRelativeRotation(FRotator(ShieldOpen>0 || Windup>0 ? 50 : 90,0,0));
    if(Weapon) Weapon->SetRelativeRotation(FRotator(0,Windup>0 ? -55.f*(1-Windup/WindupMax) : 0,0));
    if (Windup > 0)
    {
        if(Boss && (AttackPattern-1)%3==2 && Windup>.5f) { AttackTarget=P->GetActorLocation(); AttackTarget.Z=8; }
        Windup -= Dt;
        Warning->SetWorldLocation(FVector(AttackTarget.X,AttackTarget.Y,9));
        Warning->SetWorldScale3D(FVector(AttackRadius/50,AttackRadius/50,.035));
        Warning->SetVisibility(true);
        if (Windup <= 0)
        {
            Warning->SetVisibility(false);
            G->Burst(AttackTarget,AttackRadius,FLinearColor(1,.25,.12),.32);
            if (FVector::Dist2D(P->GetActorLocation(),AttackTarget) < AttackRadius+20) P->ReceiveHit(Boss ? 28 : Ranged ? 17 : 15);
            AttackTimer = Boss ? (Enraged ? 1.15f : 1.8f) : 2.0f;
            ShieldOpen=1;
        }
        return;
    }
    AttackTimer -= Dt;
    float Range = Boss ? 850 : Ranged ? 800 : 205;
    if (Delta.Size2D() > (Ranged ? 550 : 125) || (Ranged && Delta.Size2D()<350))
    {
        FVector Direction = Delta.GetSafeNormal2D()*(Ranged && Delta.Size2D()<350 ? -1 : 1);
        for (auto& Other : G->Enemies)
            if (IsValid(Other) && Other != this && !Other->Dead)
            {
                FVector Apart = GetActorLocation()-Other->GetActorLocation();
                if (Apart.SizeSquared2D() < 18000) Direction += Apart.GetSafeNormal2D()*.8f;
            }
        const float Speed = (Boss ? 170 : Armored ? 125 : Ranged ? 145 : 205) * (SlowTimer > 0 ? .35 : 1);
        FVector Next = GetActorLocation() + Direction.GetSafeNormal2D()*Speed*Dt;
        Next.X=FMath::Clamp(Next.X,-1500.f,1500.f); Next.Y=FMath::Clamp(Next.Y,-1500.f,1500.f);
        SetActorLocation(Next);
    }
    if (Delta.Size2D() < Range && AttackTimer <= 0)
    {
        WindupMax = Boss ? (Enraged ? .95f : 1.35f) : Ranged ? 1.1 : .85;
        if(G->Save->Assistance) WindupMax*=1.25f;
        Windup = WindupMax;
        AttackTarget = P->GetActorLocation(); AttackTarget.Z=8;
        if(Boss)
        {
            const int Pattern=AttackPattern++%3;
            if(Pattern==0) { AttackTarget=GetActorLocation(); AttackTarget.Z=8; AttackRadius=430; AttackName=TEXT("BRONZE NOVA / STEP BACK"); }
            else if(Pattern==1) { AttackRadius=240; AttackName=TEXT("SEA'S JUDGMENT / KEEP MOVING"); }
            else { AttackRadius=180; AttackName=TEXT("OATHBREAKER'S PURSUIT / DODGE"); }
            G->Notify(AttackName);
        }
    }
}

void ABGEnemy::Interrupt(float Duration)
{
    if(Boss || Dead) return;
    Windup=0; Warning->SetVisibility(false); StaggerTimer=Duration; AttackTimer=1; ShieldOpen=Duration+1;
}

void ABGEnemy::Hit(float Amount,const FVector& Push,bool Melee)
{
    if (Dead) return;
    auto* G = GetWorld()->GetAuthGameMode<ABGGameMode>();
    if(Armored && Melee && ShieldOpen<=0 && Windup<=0)
    {
        Amount*=.35f;
        G->FloatText(GetActorLocation()+FVector(0,0,210),TEXT("GUARDED / FINISH THE COMBO"),FLinearColor(.95,.8,.4));
    }
    Health -= Amount; FlashTimer=.15;
    if(Boss && !Enraged && Health>0 && Health<=MaxHealth*.5f)
    {
        Enraged=true;
        G->Notify(TEXT("TALOS: YOU WEAR THEIR GIFTS, MORTAL. DO YOU THINK THEY WILL LET YOU LEAVE?"));
        G->Burst(GetActorLocation(),350,FLinearColor(1,.3,.12),.6);
        if(OwnMaterial) OwnMaterial->SetScalarParameterValue(TEXT("Glow"),.35);
    }
    OnHitPresentation(Amount);
    if (!Boss) AddActorWorldOffset(Push);
    G->FloatText(GetActorLocation()+FVector(0,0,Boss ? 320 : 155),FString::FromInt(FMath::RoundToInt(Amount)),FLinearColor(.98,.86,.55));
    if (Health <= 0)
    {
        Dead=true; Warning->SetVisibility(false);
        G->Burst(GetActorLocation(),Boss ? 300 : 100,FLinearColor(.9,.64,.25),.5);
        G->EnemyKilled(this);
        Destroy();
    }
}
