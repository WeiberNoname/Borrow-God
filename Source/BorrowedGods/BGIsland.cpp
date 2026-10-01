#include "BGGameMode.h"
#include "BGVisuals.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMeshActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void ABGGameMode::BuildIsland()
{
    auto* Pale=Material(FLinearColor(.26,.32,.33));
    auto* Dark=Material(FLinearColor(.055,.095,.115));
    auto* Cliff=Material(FLinearColor(.065,.115,.13));
    auto* Light=Material(FLinearColor(.035,.65,.63),2);
    auto* Warm=Material(FLinearColor(1,.26,.045),3);
    auto* Water=UMaterialInstanceDynamic::Create(WaterMaterial ? WaterMaterial.Get() : BaseMaterial.Get(),this);
    Water->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.018,.13,.16));
    Water->SetScalarParameterValue(TEXT("Glow"),.3);
    Shape(CubeMesh,FVector(0,0,-185),FVector(240,240,.2),Water);
    Shape(CylinderMesh,FVector(0,0,-105),FVector(50,50,1.6),Cliff);
    // Preserve the original walkable floor and bounds; all additional props are cosmetic.
    Shape(CubeMesh,FVector(0,0,-27),FVector(33,33,.5),Dark,true);
    FRandomStream Random(421);
    TArray<UMaterialInstanceDynamic*> Tiles;
    for(int32 I=0;I<6;++I) Tiles.Add(Material(FLinearColor(.135,.195,.215)*( .9f+I*.035f)));
    for(int32 X=-5;X<=5;++X) for(int32 Y=-5;Y<=5;++Y)
    {
        Shape(CubeMesh,FVector(X*280,Y*280,-1),FVector(2.79,2.79,.075),Tiles[Random.RandRange(0,5)]);
    }
    // Bronze compass inlaid into the plaza, with a turquoise sanctuary seal.
    Shape(CylinderMesh,FVector(0,0,4),FVector(12,12,.035),Dark);
    BGVisuals::Ring(GetWorld(),FVector(0,0,7),610,5,GoldMaterial);
    BGVisuals::Ring(GetWorld(),FVector(0,0,8),548,2,Pale);
    BGVisuals::Ring(GetWorld(),FVector(0,0,9),230,3,Light);
    BGVisuals::Ring(GetWorld(),FVector(0,0,9),198,3,GoldMaterial);
    for(int32 I=0;I<24;++I)
    {
        float A=I*2*PI/24;
        FVector P(FMath::Cos(A)*578,FMath::Sin(A)*578,9);
        Shape(CubeMesh,P,FVector(I%3==0 ? .44f : .18f,.035,.018),GoldMaterial,false,FRotator(0,FMath::RadiansToDegrees(A),0));
    }
    // A three-pronged seal, readable at the gameplay camera height.
    Shape(CubeMesh,FVector(0,0,10),FVector(.08,2.7,.025),Light);
    Shape(CubeMesh,FVector(0,25,10),FVector(1.4,.08,.025),Light);
    for(int32 Sign : {-1,1}) Shape(CubeMesh,FVector(Sign*66,67,10),FVector(.08,.9,.025),Light);
    // Frame the square with shallow marble steps and a repeating bronze edge.
    for(int32 Side=0;Side<4;++Side)
    {
        float Yaw=Side*90;
        FVector P=FRotator(0,Yaw,0).RotateVector(FVector(0,1610,-18));
        Shape(CubeMesh,P,FVector(33,.75,.32),Pale,false,FRotator(0,Yaw,0));
        P.Z=0; Shape(CubeMesh,P,FVector(33,.045,.025),GoldMaterial,false,FRotator(0,Yaw,0));
        P=FRotator(0,Yaw,0).RotateVector(FVector(0,1690,-40));
        Shape(CubeMesh,P,FVector(34,.9,.35),StoneMaterial,false,FRotator(0,Yaw,0));
    }
    // Instanced Greek key frieze: one draw batch for the many bronze inlays.
    auto* Frieze=GetWorld()->SpawnActor<AActor>();
    auto* Inlay=NewObject<UInstancedStaticMeshComponent>(Frieze);
    Frieze->SetRootComponent(Inlay); Frieze->AddInstanceComponent(Inlay); Inlay->SetStaticMesh(CubeMesh); Inlay->SetMaterial(0,GoldMaterial);
    Inlay->SetCollisionEnabled(ECollisionEnabled::NoCollision); Inlay->SetCastShadow(false); Inlay->RegisterComponent();
    for(int32 Side=0;Side<4;++Side) for(int32 I=0;I<17;++I)
    {
        const FRotator Turn(0,Side*90,0);
        const FVector Center(-1440+I*180,1515,7);
        const FVector Offsets[]={FVector(-65,0,0),FVector(0,-35,0),FVector(45,0,0),FVector(0,35,0)};
        const FVector Scales[]={FVector(.035,.75,.016),FVector(1.3,.035,.016),FVector(.035,.75,.016),FVector(.90,.035,.016)};
        for(int32 K=0;K<4;++K) Inlay->AddInstance(FTransform(Turn,Turn.RotateVector(Center+Offsets[K]),Scales[K]));
    }
    auto Column=[&](FVector P,float Height,bool Broken)
    {
        Shape(CubeMesh,P+FVector(0,0,18),FVector(1.65,1.65,.36),Dark);
        Shape(CylinderMesh,P+FVector(0,0,43),FVector(1.22,1.22,.16),GoldMaterial);
        Shape(CylinderMesh,P+FVector(0,0,Height*.5f+50),FVector(.9,.9,Height/100),Pale);
        // Narrow flutes give a classical silhouette and catch directional shadows.
        for(int32 I=0;I<8;++I)
        {
            float A=I*2*PI/8;
            Shape(CylinderMesh,P+FVector(FMath::Cos(A)*41,FMath::Sin(A)*41,Height*.5f+50),FVector(.13,.13,Height/100),StoneMaterial);
        }
        if(!Broken)
        {
            Shape(CylinderMesh,P+FVector(0,0,Height+59),FVector(1.16,1.16,.2),GoldMaterial);
            Shape(CubeMesh,P+FVector(0,0,Height+79),FVector(1.5,1.5,.23),Pale);
        }
        else BGVisuals::Rock(GetWorld(),P+FVector(0,0,Height+44),FVector(46,46,20),Pale,static_cast<int32>(Height));
    };
    for(int32 Side=0;Side<2;++Side) for(int32 I=0;I<4;++I)
    {
        FVector P( Side==0 ? -1770 : 1770,-1050+I*700,0);
        Column(P,I==1 && Side==0 ? 135 : 420,I==1 && Side==0);
    }
    // Guardian gate moved behind the fighting space; it frames rather than hides enemies.
    for(int32 Sign : {-1,1}) Column(FVector(Sign*440,1820,0),590,false);
    Shape(CubeMesh,FVector(0,1820,695),FVector(10.5,1.8,.64),Pale);
    Shape(CubeMesh,FVector(0,1725,696),FVector(10.4,.07,.11),GoldMaterial);
    Shape(CubeMesh,FVector(0,1820,739),FVector(11.4,2.05,.2),StoneMaterial);
    Shape(SphereMesh,FVector(0,1723,698),FVector(.6,.08,.6),Light);
    int32 SideCounter=0;
    for(int32 Sign : {-1,1})
    {
        // A collapsed lintel and fragments outside the clear combat area.
        Shape(CylinderMesh,FVector(Sign*1930,-500,15),FVector(.95,.95,3.4),Pale,false,FRotator(0,20,90));
        for(int32 I=0;I<7;++I) BGVisuals::Rock(GetWorld(),FVector(Sign*(1760+Random.FRandRange(0,330)),-1250+I*410,-35),FVector(70,90,Random.FRandRange(35,115)),Cliff,SideCounter++);
    }
    // Faceted coastal rock, foam ribbons, and distant islets.
    for(int32 I=0;I<26;++I)
    {
        float A=I*2*PI/26;
        FVector P(FMath::Cos(A)*2370,FMath::Sin(A)*2370,-175);
        BGVisuals::Rock(GetWorld(),P,FVector(270,210,Random.FRandRange(80,240)),Cliff,100+I);
    }
    BGVisuals::Ring(GetWorld(),FVector(0,0,-172),2630,9,Material(FLinearColor(.16,.36,.37),.5));
    BGVisuals::Ring(GetWorld(),FVector(0,0,-173),2770,4,Material(FLinearColor(.09,.25,.28),.35));
    for(int32 I=0;I<6;++I) BGVisuals::Rock(GetWorld(),FVector(-4200+I*1800,4200+(I%2)*1400,-170),FVector(500,370,250+I*40),Cliff,200+I);
    // Bronze bowls and their warm light anchor the arena corners.
    for(int32 X : {-1,1}) for(int32 Y : {-1,1})
    {
        FVector P(X*1470,Y*1470,0);
        Shape(CubeMesh,P+FVector(0,0,25),FVector(.85,.85,.5),Dark);
        Shape(CylinderMesh,P+FVector(0,0,65),FVector(.34,.34,.7),GoldMaterial);
        Shape(SphereMesh,P+FVector(0,0,107),FVector(.85,.85,.3),GoldMaterial);
        Shape(SphereMesh,P+FVector(0,0,132),FVector(.26,.26,.65),Warm);
        auto* Fire=GetWorld()->SpawnActor<APointLight>(P+FVector(0,0,180),FRotator::ZeroRotator);
        auto* C=Fire->PointLightComponent.Get(); C->SetMobility(EComponentMobility::Movable);
        C->SetIntensity(11000); C->SetAttenuationRadius(490); C->SetLightColor(FLinearColor(1,.40,.12)); C->SetCastShadows(false);
    }
    // The Argo is visible off the southwest dock, matching the extraction objective.
    auto* Wood=Material(FLinearColor(.13,.07,.035));
    Shape(CubeMesh,FVector(-1600,-1600,-30),FVector(2.3,6.6,.25),Wood,false,FRotator(0,-45,0));
    Shape(SphereMesh,FVector(-1990,-1990,-75),FVector(3.1,8.4,1.1),Wood,false,FRotator(0,-45,0));
    Shape(CubeMesh,FVector(-1990,-1990,-26),FVector(2.4,6.6,.1),GoldMaterial,false,FRotator(0,-45,0));
    Shape(CylinderMesh,FVector(-1990,-1990,130),FVector(.12,.12,3.2),Wood);
    Shape(CubeMesh,FVector(-1990,-1990,225),FVector(2.5,.035,2.15),Material(FLinearColor(.56,.49,.34)),false,FRotator(0,-45,0));
    ExitMarker=BGVisuals::Ring(GetWorld(),FVector(-1100,-1100,12),175,12,Material(FLinearColor(.1,.85,.6),3));
    ExitMarker->SetActorHiddenInGame(true);
    HazardDisc=BGVisuals::Ring(GetWorld(),FVector(0,0,12),260,9,WarningMaterial);
    HazardDisc->SetActorHiddenInGame(true);
    auto* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,900),FRotator(-38,-28,0));
    Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sun->GetLightComponent()->SetIntensity(2.3);
    Sun->GetLightComponent()->SetLightColor(FLinearColor(1,.80,.57));
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())->LightSourceAngle=3;
    auto* Sky=GetWorld()->SpawnActor<ASkyLight>();
    auto* SkyC=Sky->GetLightComponent(); SkyC->SetMobility(EComponentMobility::Movable);
    SkyC->SourceType=SLS_SpecifiedCubemap; SkyC->SetCubemap(AmbientCube);
    SkyC->SetIntensity(.7); SkyC->SetLightColor(FLinearColor(.38,.65,.82));
    auto* Fog=GetWorld()->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.012); Fog->GetComponent()->SetFogHeightFalloff(.15);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.035,.09,.12));
    Fog->SetActorLocation(FVector(0,0,-230));
    auto* PP=GetWorld()->SpawnActor<APostProcessVolume>(); PP->bUnbound=true;
    auto& S=PP->Settings;
    S.bOverride_AutoExposureMethod=true; S.AutoExposureMethod=EAutoExposureMethod::AEM_Manual;
    S.bOverride_AutoExposureBias=true; S.AutoExposureBias=.65;
    S.bOverride_AutoExposureApplyPhysicalCameraExposure=true; S.AutoExposureApplyPhysicalCameraExposure=false;
    S.bOverride_VignetteIntensity=true; S.VignetteIntensity=.24;
    S.bOverride_BloomIntensity=true; S.BloomIntensity=.35;
    S.bOverride_AmbientOcclusionIntensity=true; S.AmbientOcclusionIntensity=.8;
    S.bOverride_AmbientOcclusionRadius=true; S.AmbientOcclusionRadius=130;
    S.bOverride_ColorSaturation=true; S.ColorSaturation=FVector4(.90,.95,1,1);
}
