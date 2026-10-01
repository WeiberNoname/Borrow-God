#pragma once
#include "CoreMinimal.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class USceneComponent;
class UStaticMesh;
class UMaterialInterface;

// Presentation-only geometry. None of these props changes navigation or damage rules.
namespace BGVisuals
{
    void RingMesh(UProceduralMeshComponent* Mesh, float Radius, float Width, int32 Segments = 64);
    AActor* Ring(UWorld* World, FVector Position, float Radius, float Width, UMaterialInterface* Material);
    AActor* Rock(UWorld* World, FVector Position, FVector Size, UMaterialInterface* Material, int32 Seed);
    UStaticMeshComponent* Part(AActor* Owner, USceneComponent* Parent, UStaticMesh* Mesh,
        FVector Position, FVector Scale, UMaterialInterface* Material, FRotator Rotation = FRotator::ZeroRotator);
}
