#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

UCLASS()
class BORROWEDGODS_API ABGPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ABGPlayerController();
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;
    virtual bool InputKey(const FInputKeyEventArgs& Params) override;
    FName Rebinding;
    bool UsingGamepad = false;
    static TArray<FName> BindingActions();
    static FKey DefaultKey(FName Action);
    FKey BoundKey(FName Action) const;
    FString KeyLabel(FName Action) const;
    void BeginRebind(FName Action);
    void ResetBindings();
};
