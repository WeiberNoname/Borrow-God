#include "BGGameMode.h"
#include "BGCharacter.h"
#include "BGEnemy.h"
#include "BGHUD.h"
#include "BGSaveGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif

// Opt-in art review. Uses the automation profile and leaves the player's save untouched.
void ABGGameMode::TickVisualPreview(float Dt)
{
#if WITH_EDITOR
    if(GShaderCompilingManager && GShaderCompilingManager->IsCompiling()) GShaderCompilingManager->FinishAllCompilation();
#endif
    Player()->Invulnerable=10; Player()->Health=Player()->MaxHealth;
    Wrath=0; EncounterDelay=0; ToastLife=0;
    for(auto& Enemy:Enemies) if(IsValid(Enemy)) Enemy->SetActorTickEnabled(false);
    SmokeTime+=Dt; if(SmokeTime<3) return; SmokeTime=0;
    auto Shot=[&](const FString& Name)
    { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots")/Name,true,false); };
    auto* Boom=Player()->CameraBoom.Get();
    auto* Hud=GetWorld()->GetFirstPlayerController()->GetHUD<ABGHUD>();
    switch(SmokeStep++)
    {
    case 0:
        Save->SelectedGod=0; StartExpedition(); Depart(); SpawnEncounter();
        Player()->SetActorLocation(FVector(0,-300,100)); Boom->bEnableCameraLag=false;
        break;
    case 1: Shot(TEXT("24-Graphics-Combat.png")); break;
    case 2:
        Hud->bShowHUD=false; Player()->SetActorLocation(FVector(0,0,100));
        Player()->Facing=FVector(1,-.5,0).GetSafeNormal(); Boom->TargetArmLength=1000;
        break;
    case 3: Shot(TEXT("25-Captain.png")); break;
    case 4:
        for(auto& Enemy:Enemies) if(IsValid(Enemy)) Enemy->Destroy(); Enemies.Empty();
        Player()->SetActorHiddenInGame(true); Player()->SetActorLocation(FVector(0,0,100));
        Boom->TargetArmLength=1850;
        for(int32 I=0;I<4;++I)
        {
            auto* E=GetWorld()->SpawnActor<ABGEnemy>(FVector(-390+I*260,150,5),FRotator(0,-90,0));
            E->Configure(I==3,I==1,I==2); E->SetActorTickEnabled(false); Enemies.Add(E);
        }
        break;
    case 5: Shot(TEXT("26-Enemy-Silhouettes.png")); break;
    case 6:
        for(auto& Enemy:Enemies) if(IsValid(Enemy)) Enemy->Destroy(); Enemies.Empty();
        Boom->TargetArmLength=5900; Boom->SetRelativeRotation(FRotator(-58,-45,0));
        Player()->SetActorLocation(FVector(0,0,100));
        break;
    case 7: Shot(TEXT("27-Sanctuary.png")); break;
    case 8:
        Hud->bShowHUD=true; Phase=EBGPhase::Victory; ReturnToHarbor(); break;
    case 9: Shot(TEXT("28-Graphics-Harbor.png")); break;
    case 10: FPlatformMisc::RequestExit(false); break;
    }
}
