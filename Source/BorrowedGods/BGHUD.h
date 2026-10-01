#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BGHUD.generated.h"

class ABGGameMode;
class UFont;
UCLASS()
class BORROWEDGODS_API ABGHUD : public AHUD
{
    GENERATED_BODY()
public:
    ABGHUD();
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;
    UPROPERTY() TObjectPtr<UFont> Regular;
    UPROPERTY() TObjectPtr<UFont> Bold;
    float S = 1;
    float OX = 0;
    float OY = 0;
    FVector2D Mouse;
    ABGGameMode* G = nullptr;
    TArray<FName> MenuActions;
    FName FocusAction;
    FName FocusContext;
    void MoveFocus(int32 Direction);
    void ActivateFocus();
    void Focusable(FName ID,float X,float Y,float W,float H);
    FLinearColor Ink = FLinearColor(0.019f,0.029f,0.038f,1.f);
    FLinearColor Panel = FLinearColor(0.035f,0.052f,0.061f,1.f);
    FLinearColor Gold = FLinearColor(0.86f,0.70f,0.40f);
    FLinearColor White = FLinearColor(0.92f,0.91f,0.85f);
    FLinearColor Muted = FLinearColor(0.48f,0.58f,0.60f);
    FLinearColor Teal = FLinearColor(0.30f,0.83f,0.77f);
    FLinearColor Red = FLinearColor(1.f,0.39f,0.32f);
    void Rect(float X,float Y,float W,float H,FLinearColor Color);
    void Line(float X,float Y,float X2,float Y2,FLinearColor Color,float Width=1);
    void Border(float X,float Y,float W,float H,FLinearColor Color);
    void Text(const FString& Value,float X,float Y,float Size,FLinearColor Color,bool Strong=false,bool Center=false);
    void Wrap(const FString& Value,float X,float Y,float W,float Size,FLinearColor Color,float Spacing=1.45f);
    void Diamond(float X,float Y,float R,FLinearColor Color);
    void Icon(int32 Type,float X,float Y,float R,FLinearColor Color);
    void Button(FName ID,const FString& Label,float X,float Y,float W,float H,bool Primary=false,bool Enabled=true);
    void Bar(float X,float Y,float W,float H,float Fraction,FLinearColor Color);
    void Header(const FString& Section);
    void Harbor();
    void Combat();
    void Offering();
    void Result();
    void Journal();
    void Pause();
    void Help();
    void Crew();
    void Decision();
    void Settings();
};
