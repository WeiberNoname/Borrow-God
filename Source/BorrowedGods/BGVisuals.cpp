#include "BGVisuals.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

void BGVisuals::RingMesh(UProceduralMeshComponent* Mesh, float Radius, float Width, int32 Segments)
{
    TArray<FVector> Vertices, Normals;
    TArray<int32> Triangles;
    TArray<FVector2D> UV;
    for(int32 I=0;I<=Segments;++I)
    {
        const float A=I*2*PI/Segments;
        for(float R : {Radius-Width,Radius})
        {
            Vertices.Add(FVector(FMath::Cos(A)*R,FMath::Sin(A)*R,0));
            Normals.Add(FVector::UpVector); UV.Add(FVector2D(I/static_cast<float>(Segments),R==Radius ? 1 : 0));
        }
        if(I<Segments) { int32 K=I*2; Triangles.Append({K,K+3,K+1,K,K+2,K+3}); }
    }
    Mesh->CreateMeshSection_LinearColor(0,Vertices,Triangles,Normals,UV,TArray<FLinearColor>(),TArray<FProcMeshTangent>(),false);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); Mesh->SetCastShadow(false);
}
AActor* BGVisuals::Ring(UWorld* World,FVector Position,float Radius,float Width,UMaterialInterface* Material)
{
    auto* Actor=World->SpawnActor<AActor>(Position,FRotator::ZeroRotator);
    auto* Mesh=NewObject<UProceduralMeshComponent>(Actor);
    Actor->SetRootComponent(Mesh); Actor->AddInstanceComponent(Mesh); Mesh->RegisterComponent();
    RingMesh(Mesh,Radius,Width); Mesh->SetMaterial(0,Material);
    Actor->SetActorLocation(Position); return Actor;
}
AActor* BGVisuals::Rock(UWorld* World,FVector Position,FVector Size,UMaterialInterface* Material,int32 Seed)
{
    auto* Actor=World->SpawnActor<AActor>(Position,FRotator(0,Seed*31,0));
    auto* Mesh=NewObject<UProceduralMeshComponent>(Actor);
    Actor->SetRootComponent(Mesh); Actor->AddInstanceComponent(Mesh); Mesh->RegisterComponent();
    FRandomStream Random(Seed); TArray<FVector> Points;
    for(int32 Layer=0;Layer<3;++Layer) for(int32 I=0;I<7;++I)
    {
        float A=I*2*PI/7+Layer*.15f;
        float R=(Layer==1 ? 1.f : Layer==2 ? .65f : .90f)*Random.FRandRange(.8f,1.1f);
        Points.Add(FVector(FMath::Cos(A)*Size.X*R,FMath::Sin(A)*Size.Y*R,Size.Z*(Layer*.5f)+Random.FRandRange(-15,15)));
    }
    Points.Add(FVector(0,0,Size.Z*1.05f));
    TArray<FVector> Vertices,Normals; TArray<int32> Triangles; TArray<FVector2D> UV;
    auto Triangle=[&](int32 A,int32 B,int32 C)
    {
        const FVector N=FVector::CrossProduct(Points[B]-Points[A],Points[C]-Points[A]).GetSafeNormal();
        for(int32 K : {A,C,B}) { Triangles.Add(Vertices.Num()); Vertices.Add(Points[K]); Normals.Add(N); UV.Add(FVector2D(Points[K].X,Points[K].Y)/100); }
    };
    for(int32 I=0;I<7;++I)
    {
        int32 N=(I+1)%7;
        for(int32 L=0;L<2;++L) { int32 K=L*7; Triangle(K+I,K+N,K+N+7); Triangle(K+I,K+N+7,K+I+7); }
        Triangle(14+I,14+N,21);
    }
    Mesh->CreateMeshSection_LinearColor(0,Vertices,Triangles,Normals,UV,TArray<FLinearColor>(),TArray<FProcMeshTangent>(),false);
    Mesh->SetMaterial(0,Material); Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Actor->SetActorLocation(Position); Actor->SetActorRotation(FRotator(0,Seed*31,0)); return Actor;
}
UStaticMeshComponent* BGVisuals::Part(AActor* Owner,USceneComponent* Parent,UStaticMesh* Mesh,FVector Position,FVector Scale,UMaterialInterface* Material,FRotator Rotation)
{
    auto* Part=NewObject<UStaticMeshComponent>(Owner);
    Owner->AddInstanceComponent(Part); Part->SetupAttachment(Parent); Part->SetStaticMesh(Mesh);
    Part->SetMaterial(0,Material); Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Part->SetRelativeLocation(Position); Part->SetRelativeScale3D(Scale); Part->SetRelativeRotation(Rotation);
    Part->RegisterComponent(); return Part;
}
