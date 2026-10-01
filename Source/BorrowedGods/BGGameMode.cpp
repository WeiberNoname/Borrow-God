#include "BGGameMode.h"
#include "BGCharacter.h"
#include "BGEnemy.h"
#include "BGHUD.h"
#include "BGPlayerController.h"
#include "BGSaveGame.h"
#include "BGVisuals.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMisc.h"
#include "HighResScreenshot.h"
#include "InputKeyEventArgs.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "GameFramework/SpringArmComponent.h"

ABGGameMode::ABGGameMode()
{
    PrimaryActorTick.bCanEverTick=true;
    DefaultPawnClass=ABGCharacter::StaticClass();
    PlayerControllerClass=ABGPlayerController::StaticClass();
    HUDClass=ABGHUD::StaticClass();
    EnemyClass=ABGEnemy::StaticClass(); GuardianClass=ABGEnemy::StaticClass();
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    CubeMesh=Cube.Object; SphereMesh=Sphere.Object; CylinderMesh=Cylinder.Object;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Prototype/M_SanctuaryStone_v2.M_SanctuaryStone_v2"));
    BaseMaterial=Mat.Object;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Water(TEXT("/Game/Prototype/M_AegeanWater_v3.M_AegeanWater_v3"));
    WaterMaterial=Water.Object;
    static ConstructorHelpers::FObjectFinder<UTextureCube> Ambient(TEXT("/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube"));
    AmbientCube=Ambient.Object;
}

void ABGGameMode::BeginPlay()
{
    Super::BeginPlay();
    VisualPreview=FParse::Param(FCommandLine::Get(),TEXT("BGVisualPreview"));
    SmokeTest=VisualPreview || FParse::Param(FCommandLine::Get(),TEXT("BGSmokeTest"));
    if (!SmokeTest && UGameplayStatics::DoesSaveGameExist(SaveSlot(),0)) Save=Cast<UBGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot(),0));
    if (!Save) Save=Cast<UBGSaveGame>(UGameplayStatics::CreateSaveGameObject(UBGSaveGame::StaticClass()));
    Save->HullLevel=FMath::Clamp(Save->HullLevel,0,5); Save->BladeLevel=FMath::Clamp(Save->BladeLevel,0,5);
    Save->SelectedGod=FMath::Clamp(Save->SelectedGod,0,2); Save->Obols=FMath::Max(0,Save->Obols);
    Save->HomewardProgress=FMath::Clamp(Save->HomewardProgress,0,3);
    Save->SailorRecovery=FMath::Clamp(Save->SailorRecovery,0,2);
    God=static_cast<EBGGod>(Save->SelectedGod);
    BorrowedGods=1<<Save->SelectedGod;
    StoneMaterial=Material(FLinearColor(.11,.17,.20));
    GoldMaterial=Material(FLinearColor(.56,.31,.10),.03,.34,.65);
    PlayerMaterial=Material(FLinearColor(.12,.65,.61),.12);
    EnemyMaterial=Material(FLinearColor(.48,.18,.14));
    WarningMaterial=Material(FLinearColor(1,.10,.025),3);
    BuildIsland();
    for(int32 I=0;I<4;++I)
    {
        auto* Voice=NewObject<UAudioComponent>(this);
        Voice->bAutoActivate=false; Voice->bIsUISound=true; Voice->RegisterComponent();
        AudioVoices.Add(Voice); AudioWaves.Add(nullptr);
    }
    if (auto* P=Player()) { P->ResetForRun(); P->SetActorHiddenInGame(true); }
    Notify(TEXT("Welcome aboard, oathbreaker. Choose the power you will steal."));
}

ABGCharacter* ABGGameMode::Player() const { return Cast<ABGCharacter>(UGameplayStatics::GetPlayerPawn(this,0)); }
bool ABGGameMode::IsPlaying() const { return Phase==EBGPhase::Combat && !Paused && !JournalOpen && !HelpOpen && !CrewOpen && !SettingsOpen && HitStop<=0; }
bool ABGGameMode::HasActiveVoyage() const { return Phase!=EBGPhase::Harbor && Phase!=EBGPhase::Briefing && Phase!=EBGPhase::Victory && Phase!=EBGPhase::Defeat; }
bool ABGGameMode::HasGod(EBGGod Value) const { return (BorrowedGods & (1<<static_cast<int32>(Value)))!=0; }
int32 ABGGameMode::DebtCount() const { int32 Count=0; for(int32 I=0;I<3;++I) if(HasGod(static_cast<EBGGod>(I))) ++Count; return Count; }
void ABGGameMode::SwitchGod(int32 Index)
{
    if(!IsPlaying() || Index<0 || Index>2 || !HasGod(static_cast<EBGGod>(Index))) return;
    God=static_cast<EBGGod>(Index); Player()->RefreshPower();
    PlayerMaterial->SetVectorParameterValue(TEXT("Tint"),BGRules::GodColor(God)*.65f);
}
void ABGGameMode::CycleGod()
{
    for(int32 Step=1;Step<=3;++Step) { int32 Index=(static_cast<int32>(God)+Step)%3; if(HasGod(static_cast<EBGGod>(Index))) { SwitchGod(Index); return; } }
}
FString ABGGameMode::BurdenSummary() const
{
    TArray<FString> Parts;
    if(HasGod(EBGGod::Poseidon)) Parts.Add(TEXT("FASTER WRATH"));
    if(HasGod(EBGGod::Zeus)) Parts.Add(TEXT("ALL CASTS COST 8 HP"));
    if(HasGod(EBGGod::Hades)) Parts.Add(TEXT("75 MAX STAMINA"));
    return FString::Join(Parts,TEXT(" / "));
}
TArray<int32> ABGGameMode::OfferingOptions() const
{
    TArray<int32> Options;
    for(int32 I=0;I<3;++I) if(!HasGod(static_cast<EBGGod>(I))) Options.Add(I);
    // There is always a path that refuses another god. Full loadouts receive mortal blessings.
    if(Options.Num()==2) Options.Add(3);
    else { Options.Add(4); Options.Add(3); if(Options.Num()<3) Options.Add(5); }
    return Options;
}
FString ABGGameMode::CrewDialogue() const
{
    const int32 Progress=Save->HomewardProgress;
    if(CrewSpeaker==0)
    {
        if(Progress==0) return TEXT("My daughter was six when we left. I keep counting the years by the stars, but these stars never move. Talos guards a piece of the old sea chart. Bring it aboard, captain. Give me one honest direction.");
        if(Progress==1) return TEXT("The first relic moved a star on my chart. Just one. For the first time since we were stranded, I can see where the sea bends. Two more fragments and I can draw a course. I have not told the crew yet. I cannot bear to give them hope and take it back.");
        if(Progress==2) return TEXT("I found our coast between the fragments. The others are sleeping, so I will say it to you first: home is real. One more relic will give us the bearing. Do not let another god convince you the ship is worth less than their gift.");
        return TEXT("There. The three fragments fit. That small mark is our coast, not another temple. We finally have a course home. This sanctuary has given us everything it can. Let us remember the people who waited, before we decide what comes next.");
    }
    if(CrewSpeaker==1)
    {
        if(!Save->LastConsequence.IsEmpty())
        {
            if(!Save->LastVoyageEscaped) return Save->SailorRecovery>=2 ? TEXT("Tomas helped me pull you aboard. He kept saying you would come back. We lost the medicine, but I am glad I did not have to tell him otherwise.") : TEXT("We pulled you from the water, captain. The medicine did not come with you. Tomas is still waiting. Rest tonight. Tomorrow, we try again.");
            if(Save->LastMedicine==0) return Save->SailorRecovery>=2 ? TEXT("Tomas is on deck again, thanks to what you brought before. There was no medicine this time. I hope the miracle was worth it; he will not be our last wounded sailor.") : TEXT("I saw the new power in your hands. There was no medicine in them. Tomas is still burning with fever. I will stay beside him. Next time, leave room for something a god cannot use.");
            if(Save->SailorRecovery>=2) return Save->LastTribute ? TEXT("Tomas is back on deck. He wants to thank you himself. You took the harder path and still left him a dose after the tribute. I do not need a perfect captain. I need one who brings us home.") : TEXT("Tomas is back on deck. He asked whether the sea fought you for the medicine. I told him it did, and that you came home with it anyway. He wants to thank you himself.");
            return Save->LastTribute ? TEXT("Tomas is recovering. You paid the sea and still brought him a dose. One more should put him on his feet. He keeps asking whether we can still get home.") : TEXT("Tomas is recovering. He slept through the night. One more dose should put him on his feet. He keeps asking whether we can still get home. For once, I wanted to tell him yes.");
        }
        if(Save->VoyageJournal.IsEmpty()) return TEXT("You do not have to take every gift, captain. A mortal can refuse a god. At the shrines, ask whether another power is worth another curse. I can mend a wound. I cannot mend a promise you have forgotten.");
        if(!Save->LastVoyageEscaped) return TEXT("We pulled you from the water. You were still holding your weapon. Next time, leave room for your own breath: red circles are warnings, not challenges. The obols we lost can be earned again. You cannot.");
        if(Save->LastDebtCount>=3) return TEXT("Three gods followed you aboard. I can feel them in the planks. You brought the relic home, yes. But you came back speaking with someone else's voice. Before you sail again, tell me one thing you want that no god has promised you.");
        return TEXT("You returned with the relic and your own name. That counts for something. Refusing a gift is not weakness. Sometimes it is the only way to keep enough of yourself to come home.");
    }
    return TEXT("I forge bronze, not miracles. Keep your attacks together and the third strike breaks their footing. Wet an enemy with Tidebreak, then call lightning: water carries the storm. Carry both gifts if you must, but their curses stay even when you change masks.");
}
FString ABGGameMode::SaveSlot() const { return SmokeTest ? TEXT("BorrowedGods_Automation") : TEXT("BorrowedGods_Profile_v1"); }
UMaterialInstanceDynamic* ABGGameMode::Material(FLinearColor Color,float Glow,float Roughness,float Metallic)
{
    auto* M=UMaterialInstanceDynamic::Create(BaseMaterial ? BaseMaterial.Get() : UMaterial::GetDefaultMaterial(MD_Surface),this);
    M->SetVectorParameterValue(TEXT("Tint"),Color);
    M->SetScalarParameterValue(TEXT("Glow"),Glow);
    M->SetScalarParameterValue(TEXT("Roughness"),Roughness);
    M->SetScalarParameterValue(TEXT("Metallic"),Metallic);
    return M;
}
AStaticMeshActor* ABGGameMode::Shape(UStaticMesh* Mesh,FVector Position,FVector Scale,UMaterialInterface* Mat,bool Collision,FRotator Rotation)
{
    auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(Position,Rotation);
    auto* C=A->GetStaticMeshComponent(); C->SetMobility(EComponentMobility::Movable); C->SetStaticMesh(Mesh);
    C->SetMaterial(0,Mat); C->SetCollisionEnabled(Collision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    A->SetActorScale3D(Scale); return A;
}

void ABGGameMode::StartExpedition()
{
    if (Phase!=EBGPhase::Harbor) return;
    Phase=EBGPhase::Briefing; Paused=JournalOpen=HelpOpen=CrewOpen=SettingsOpen=false;
    ToastLife=0;
}

void ABGGameMode::Depart()
{
    if(Phase!=EBGPhase::Briefing) return;
    for (auto& E:Enemies) if (IsValid(E)) E->Destroy();
    Enemies.Empty(); FloatingText.Empty();
    Phase=EBGPhase::Combat; Paused=JournalOpen=HelpOpen=CrewOpen=SettingsOpen=false;
    God=static_cast<EBGGod>(Save->SelectedGod); BorrowedGods=1<<Save->SelectedGod;
    Encounter=CarriedObols=Kills=RunDamageBonus=RunBoonCount=0;
    RunCooldownMultiplier=1; Wrath=RunTime=HazardTimer=0;
    RiskyRoute=TributeAccepted=BargainResolved=SeaStrike=false; Medicine=SeaStrikesRemaining=0;
    SeaStrikeDelay=HitStop=ImpactFeedback=DamageFeedback=0; Relic=EBGRelic::None; VoyageConsequence.Empty();
    PlayerMaterial->SetVectorParameterValue(TEXT("Tint"),BGRules::GodColor(God)*.65f);
    AwaitingExtraction=false; ExitMarker->SetActorHiddenInGame(true); HazardDisc->SetActorHiddenInGame(true);
    Player()->ResetForRun(); Player()->SetActorHiddenInGame(false);
    ++Save->Expeditions; SaveProgress();
    EncounterDelay=1.5;
    Notify(TEXT("THE DROWNED SANCTUARY  /  Clear the shore. Red circles warn of incoming attacks."));
    Feedback(5);
}

void ABGGameMode::ChooseRoute(int32 Index)
{
    if(Phase!=EBGPhase::Route || Index<0 || Index>1) return;
    RiskyRoute=Index==1;
    Phase=EBGPhase::Offering;
    Feedback(5);
}

void ABGGameMode::ChooseRelic(int32 Index)
{
    if(Phase!=EBGPhase::Relic || Index<0 || Index>2) return;
    Relic=static_cast<EBGRelic>(Index+1);
    Encounter=1; Phase=EBGPhase::Combat;
    Player()->Invulnerable=1; Player()->SetActorLocation(FVector(-600,-450,100));
    SpawnEncounter(); Feedback(5);
    Notify(RiskyRoute ? TEXT("THE GUARDED VAULT / Break the sentinels. Two medicine flasks await.") : TEXT("THE HEALER'S PATH / Clear the infirmary. One medicine flask awaits."));
}

void ABGGameMode::ChooseBargain(int32 Index)
{
    if(Phase!=EBGPhase::Bargain || Index<0 || Index>1) return;
    TributeAccepted=Index==0;
    BargainResolved=true;
    if(TributeAccepted) { Medicine=FMath::Max(0,Medicine-1); SeaStrikesRemaining=0; }
    else { SeaStrikesRemaining=3; SeaStrikeDelay=6; }
    Phase=EBGPhase::Offering; Feedback(5);
    Notify(TributeAccepted ? TEXT("TRIBUTE PAID / Your powers gain 20 damage. The flask will never reach Tomas.") : TEXT("TRIBUTE REFUSED / Keep the medicine. Three sea strikes will test your footing."));
}

void ABGGameMode::SpawnEncounter()
{
    Enemies.Empty();
    const FVector Spots[] = {FVector(400,200,5),FVector(-200,700,5),FVector(750,-450,5),FVector(-650,500,5),FVector(1050,400,5)};
    const int Count=Encounter==0 ? 3 : Encounter==1 ? (RiskyRoute ? 5 : 3) : 3;
    for (int I=0;I<Count;++I)
    {
        const bool Boss=Encounter==2 && I==0;
        auto* E=GetWorld()->SpawnActor<ABGEnemy>(Boss ? GuardianClass : EnemyClass,Spots[I],FRotator::ZeroRotator);
        E->Configure(Boss, !Boss && I%2==1, !Boss && ((Encounter==1 && RiskyRoute && I%2==0) || (Encounter==2 && I==2))); Enemies.Add(E);
    }
    Notify(Encounter==0 ? TEXT("I / THE SHORE - Defeat the oathbound") : Encounter==1 ? (RiskyRoute ? TEXT("II / THE VAULT - Sentinels guard two flasks") : TEXT("II / THE INFIRMARY - Recover medicine for Nera")) : TEXT("III / THE SANCTUM - Defeat Talos, the Hollow Guardian"));
}

void ABGGameMode::EnemyKilled(ABGEnemy* Enemy)
{
    ++Kills; CarriedObols+=Enemy->Boss ? 45 : 8;
    Enemies.Remove(Enemy);
    Wrath=FMath::Max(0.f,Wrath-4);
    if (Enemies.Num()==0) EncounterDelay=1.3;
}

void ABGGameMode::ChooseOffering(int32 Index)
{
    if (Phase!=EBGPhase::Offering || Index<0 || Index>2) return;
    const int32 Choice=OfferingOptions()[Index];
    if(Choice<=2)
    {
        BorrowedGods|=1<<Choice; God=static_cast<EBGGod>(Choice);
        if(HasGod(EBGGod::Hades)) { Player()->MaxStamina=75; Player()->Stamina=FMath::Min(75.f,Player()->Stamina); }
        Wrath=FMath::Min(100.f,Wrath+25);
        Player()->RefreshPower();
    }
    if (Choice==3) { Player()->MaxHealth+=20; Player()->Heal(45); Wrath=FMath::Max(0.f,Wrath-25); }
    if (Choice==4) RunDamageBonus+=8;
    if (Choice==5) { RunCooldownMultiplier*=.8; for(float& Cooldown:Player()->GodCooldowns) Cooldown=0; Player()->RefreshPower(); }
    ++RunBoonCount;
    if(Encounter==0) Phase=EBGPhase::Relic;
    else
    {
        ++Encounter; Phase=EBGPhase::Combat;
        Player()->Invulnerable=1; SpawnEncounter();
    }
    if(Choice<=2) Notify(BGRules::GodName(God)+TEXT(" BORROWED / Switch powers with R. Every curse stays with you."));
    else if(Choice==3) Notify(TEXT("A MORTAL'S RESOLVE / You refused another debt. Your crew would be proud."));
}

void ABGGameMode::FinishExpedition(bool Success)
{
    if (!HasActiveVoyage()) return;
    Phase=Success ? EBGPhase::Victory : EBGPhase::Defeat;
    Paused=JournalOpen=HelpOpen=CrewOpen=SettingsOpen=false;
    LastReward=BGRules::BankedReward(CarriedObols,Success);
    Save->Obols+=LastReward;
    if (Success) ++Save->Victories;
    if(Success) Save->HomewardProgress=FMath::Min(3,Save->HomewardProgress+1);
    Save->LastDebtCount=DebtCount(); Save->LastVoyageEscaped=Success;
    Save->LastMedicine=Success ? Medicine : 0; Save->LastTribute=TributeAccepted; Save->LastRiskyRoute=RiskyRoute;
    if(Success && Medicine>0)
    {
        const int32 Before=Save->SailorRecovery;
        Save->SailorRecovery=FMath::Min(2,Before+Medicine); Save->MedicineDelivered+=Medicine;
        VoyageConsequence=Before>=2 ? TEXT("Nera stores the medicine for the next wounded sailor.") : Save->SailorRecovery>=2 ? TEXT("Tomas's fever breaks. He will take his place on deck again.") : TEXT("Tomas is recovering. Nera needs one more dose to put him on his feet.");
    }
    else if(Success) VoyageConsequence=TEXT("The tribute made you stronger. Tomas received no medicine; his fever remains.");
    else VoyageConsequence=TEXT("Nera pulled you from the water. The medicine was lost at sea. Tomas is still waiting.");
    // A recovered sailor stays recovered even if later voyages bring no supplies.
    if(Save->SailorRecovery>=2 && (!Success || Medicine==0)) VoyageConsequence=TEXT("Tomas helps Nera pull you aboard. The ship's medicine reserves received nothing this time.");
    Save->LastConsequence=VoyageConsequence;
    Save->VoyageJournal.Add(FString::Printf(TEXT("Voyage %d / %s / %s / %d medicine / %d obols"),Save->Expeditions,Success ? TEXT("RELIC RECOVERED") : TEXT("RETURNED FROM THE DEEP"),RiskyRoute ? TEXT("VAULT") : TEXT("SAFE PATH"),Save->LastMedicine,LastReward));
    if(Save->VoyageJournal.Num()>6) Save->VoyageJournal.RemoveAt(0);
    SaveProgress();
    for (auto& E:Enemies) if (IsValid(E)) E->Destroy();
    Enemies.Empty(); HazardDisc->SetActorHiddenInGame(true);
    Notify(Success ? TEXT("Your crew has secured the relic. The sea remembers your theft.") : TEXT("The ferryman takes his share. A quarter of your obols survives."));
    Feedback(Success ? 5 : 4);
}

void ABGGameMode::ReturnToHarbor()
{
    if (Phase!=EBGPhase::Victory && Phase!=EBGPhase::Defeat) return;
    Phase=EBGPhase::Harbor; God=static_cast<EBGGod>(Save->SelectedGod); BorrowedGods=1<<Save->SelectedGod; Player()->SetActorHiddenInGame(true);
    Player()->ResetForRun(); AwaitingExtraction=false; ExitMarker->SetActorHiddenInGame(true);
    ToastLife=0;
}
bool ABGGameMode::Upgrade(bool Blade)
{
    if (Phase!=EBGPhase::Harbor) return false;
    int32& Level=Blade ? Save->BladeLevel : Save->HullLevel;
    const int Cost=BGRules::UpgradeCost(Level);
    if (Level>=5 || Save->Obols<Cost) { Notify(Level>=5 ? TEXT("This upgrade is complete.") : TEXT("Bring home more obols to purchase this upgrade.")); return false; }
    Save->Obols-=Cost; ++Level; SaveProgress();
    Notify(Blade ? TEXT("FORGED ANEW  /  +4 melee damage on every expedition.") : TEXT("A STRONGER OATH  /  +15 starting health on every expedition."));
    return true;
}
void ABGGameMode::SelectGod(int32 Index)
{
    if (Phase!=EBGPhase::Harbor || Index<0 || Index>2) return;
    God=static_cast<EBGGod>(Index); BorrowedGods=1<<Index; Save->SelectedGod=Index; SaveProgress();
}
void ABGGameMode::SaveProgress()
{
    SaveSucceeded=UGameplayStatics::SaveGameToSlot(Save,SaveSlot(),0);
    if (!SaveSucceeded) Notify(TEXT("Could not save progress. Check available disk space."));
}
void ABGGameMode::Interact()
{
    if (!IsPlaying() || !AwaitingExtraction) return;
    if (FVector::Dist2D(Player()->GetActorLocation(),ExitMarker->GetActorLocation())<280) FinishExpedition(true);
    else Notify(TEXT("Return to the green beacon at the southwest corner. Press E to board."));
}
void ABGGameMode::Notify(const FString& Message) { Toast=Message; ToastLife=5; }
void ABGGameMode::FloatText(const FVector& At,const FString& Value,FLinearColor Color) { FloatingText.Add({At,Value,Color,1.2f}); }
void ABGGameMode::Burst(const FVector& At,float Radius,FLinearColor Color,float Duration)
{
    auto* A=BGVisuals::Ring(GetWorld(),FVector(At.X,At.Y,13),50,4,Material(Color,2));
    Effects.Add({A,0,Duration,Radius});
}
FString ABGGameMode::Objective() const
{
    if (AwaitingExtraction) return TEXT("Return to your ship  /  E at the green beacon");
    if (Encounter==0) return TEXT("Defeat the oathbound on the shore");
    if (Encounter==1) return RiskyRoute ? TEXT("Break the vault guard / recover two flasks") : TEXT("Clear the infirmary / recover one flask");
    return TEXT("Defeat Talos and reclaim the sea relic");
}

void ABGGameMode::HandleAction(FName Action)
{
    if (Action==TEXT("pause"))
    {
        if(auto* PC=Cast<ABGPlayerController>(GetWorld()->GetFirstPlayerController()); PC && !PC->Rebinding.IsNone()) { PC->Rebinding=NAME_None; return; }
        if(SettingsOpen) { SettingsOpen=false; Paused=true; return; }
        if(Phase==EBGPhase::Briefing) { Phase=EBGPhase::Harbor; return; }
        if(CrewOpen) CrewOpen=false;
        else if (HelpOpen) HelpOpen=false;
        else if (JournalOpen) JournalOpen=false;
        else Paused=!Paused;
        return;
    }
    if(Action==TEXT("crew")) { CrewOpen=!CrewOpen; JournalOpen=Paused=HelpOpen=SettingsOpen=false; return; }
    for(int I=0;I<3;++I) if(Action==FName(*FString::Printf(TEXT("crew%d"),I))) { CrewSpeaker=I; return; }
    if (Action==TEXT("help")) { HelpOpen=!HelpOpen; JournalOpen=Paused=CrewOpen=SettingsOpen=false; return; }
    if (Action==TEXT("journal")) { JournalOpen=!JournalOpen; HelpOpen=Paused=CrewOpen=SettingsOpen=false; return; }
    if (Action==TEXT("close")) { Paused=HelpOpen=JournalOpen=CrewOpen=SettingsOpen=false; return; }
    if (Action==TEXT("settings")) { SettingsOpen=true; Paused=HelpOpen=JournalOpen=CrewOpen=false; return; }
    if (Action==TEXT("assist")) { Save->Assistance=!Save->Assistance; SaveProgress(); return; }
    if (Action==TEXT("sound")) { Save->Muted=!Save->Muted; if(Save->Muted) for(auto& Voice:AudioVoices) Voice->Stop(); SaveProgress(); return; }
    if (Action==TEXT("resetkeys")) { if(auto* PC=Cast<ABGPlayerController>(GetWorld()->GetFirstPlayerController())) PC->ResetBindings(); return; }
    if(Action.ToString().StartsWith(TEXT("bind_"))) { if(SettingsOpen) if(auto* PC=Cast<ABGPlayerController>(GetWorld()->GetFirstPlayerController())) PC->BeginRebind(FName(*Action.ToString().RightChop(5))); return; }
    if (Action==TEXT("motion")) { Save->ReducedMotion=!Save->ReducedMotion; Player()->CameraBoom->bEnableCameraLag=!Save->ReducedMotion; SaveProgress(); return; }
    if (Action==TEXT("quit")) { SaveProgress(); GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit")); return; }
    if (Action==TEXT("abandon") && HasActiveVoyage()) { FinishExpedition(false); return; }
    if (Paused || JournalOpen || HelpOpen || CrewOpen || SettingsOpen) return;
    if (Action==TEXT("sail")) StartExpedition();
    if (Action==TEXT("depart")) Depart();
    if (Action==TEXT("cancelvoyage") && Phase==EBGPhase::Briefing) Phase=EBGPhase::Harbor;
    if (Action==TEXT("harbor")) ReturnToHarbor();
    if (Action==TEXT("hull")) Upgrade(false);
    if (Action==TEXT("blade")) Upgrade(true);
    for (int I=0;I<3;++I) if (Action==FName(*FString::Printf(TEXT("choice%d"),I)))
    {
        if (Phase==EBGPhase::Harbor) SelectGod(I);
        else if(Phase==EBGPhase::Route) ChooseRoute(I);
        else if(Phase==EBGPhase::Relic) ChooseRelic(I);
        else if(Phase==EBGPhase::Bargain) ChooseBargain(I);
        else if (Phase==EBGPhase::Offering) ChooseOffering(I);
        else if(Phase==EBGPhase::Combat) SwitchGod(I);
    }
}

void ABGGameMode::Tick(float Dt)
{
    Super::Tick(Dt);
    HitStop=FMath::Max(0.f,HitStop-Dt);
    ImpactFeedback=FMath::Max(0.f,ImpactFeedback-Dt);
    DamageFeedback=FMath::Max(0.f,DamageFeedback-Dt);
    for(int I=0;I<4;++I) if(AudioTimers[I]>0) { AudioTimers[I]-=Dt; if(AudioTimers[I]<=0 && AudioVoices.IsValidIndex(I)) { AudioVoices[I]->Stop(); AudioWaves[I]=nullptr; } }
    ToastLife=FMath::Max(0.f,ToastLife-Dt);
    for (int I=FloatingText.Num()-1;I>=0;--I) { FloatingText[I].Life-=Dt; FloatingText[I].Position.Z+=Dt*45; if (FloatingText[I].Life<=0) FloatingText.RemoveAt(I); }
    for (int I=Effects.Num()-1;I>=0;--I)
    {
        auto& E=Effects[I]; E.Age+=Dt;
        if (!E.Actor.IsValid() || E.Age>=E.Duration) { if (E.Actor.IsValid()) E.Actor->Destroy(); Effects.RemoveAt(I); }
        else { float R=FMath::Lerp(.15f,1.f,E.Age/E.Duration)*E.Radius/50; E.Actor->SetActorScale3D(FVector(R,R,1)); }
    }
    if (IsPlaying())
    {
        RunTime+=Dt;
        if (!AwaitingExtraction)
        {
            if(Encounter==2 && SeaStrikesRemaining>0 && HazardTimer<=0)
            {
                SeaStrikeDelay-=Dt;
                if(SeaStrikeDelay<=0) { --SeaStrikesRemaining; SeaStrikeDelay=8; StartHazard(true); }
            }
            Wrath=FMath::Min(100.f,Wrath+Dt*((HasGod(EBGGod::Poseidon) ? 2.4f : 1.4f)+(DebtCount()-1)*.65f));
            if (HazardTimer>0)
            {
                HazardTimer-=Dt;
                if (HazardTimer<=0)
                {
                    HazardDisc->SetActorHiddenInGame(true);
                    Burst(HazardPosition,260,FLinearColor(1,.35,.1),.5);
                    if (FVector::Dist2D(Player()->GetActorLocation(),HazardPosition)<280) Player()->ReceiveHit(30);
                    if(!SeaStrike) Wrath=25;
                    SeaStrike=false;
                }
            }
            else if (Wrath>=100)
            {
                StartHazard(false);
            }
        }
        if (EncounterDelay>0 && Phase==EBGPhase::Combat)
        {
            EncounterDelay-=Dt;
            if (EncounterDelay<=0)
            {
                if (Kills==0) SpawnEncounter();
                else if (Encounter<2)
                {
                    Phase=Encounter==0 ? EBGPhase::Route : EBGPhase::Bargain;
                    if(Encounter==1) Medicine=RiskyRoute ? 2 : 1;
                    HazardTimer=0; SeaStrike=false; HazardDisc->SetActorHiddenInGame(true);
                    Feedback(5);
                }
                else { AwaitingExtraction=true; HazardTimer=0; HazardDisc->SetActorHiddenInGame(true); ExitMarker->SetActorHiddenInGame(false); Notify(TEXT("THE RELIC IS YOURS  /  Return to the green beacon. Press E to escape.")); }
            }
        }
    }
    if (VisualPreview) TickVisualPreview(Dt);
    else if (SmokeTest) TickSmokeTest(Dt);
}

void ABGGameMode::StartHazard(bool FromSea)
{
    SeaStrike=FromSea;
    HazardTimer=Save->Assistance ? 2.f : 1.6f;
    HazardPosition=Player()->GetActorLocation(); HazardPosition.Z=12;
    HazardDisc->SetActorLocation(HazardPosition); HazardDisc->SetActorHiddenInGame(false);
    Notify(FromSea ? TEXT("POSEIDON'S CLAIM / Medicine is yours. Leave the marked ground!") : TEXT("DIVINE RETRIBUTION / Leave the marked ground!"));
    Feedback(6);
}

void ABGGameMode::Impact(bool Finisher)
{
    ImpactFeedback=Finisher ? .16f : .09f;
    if(!Save->ReducedMotion) HitStop=Finisher ? .045f : .025f;
    Feedback(Finisher ? 2 : 1);
}

void ABGGameMode::Feedback(int32 Cue)
{
    if(!Save || Save->Muted || AudioVoices.Num()!=4) return;
    const int32 Index=AudioVoice++%4;
    AudioVoices[Index]->Stop();
    auto* Wave=NewObject<USoundWaveProcedural>(this);
    Wave->SetSampleRate(24000); Wave->NumChannels=1; Wave->Duration=INDEFINITELY_LOOPING_DURATION;
    Wave->bLooping=false;
    const float Duration=Cue==5 ? .36f : Cue==6 ? .28f : Cue==2 ? .19f : .12f;
    const int32 Count=FMath::RoundToInt(24000*Duration);
    TArray<int16> PCM; PCM.SetNumUninitialized(Count);
    FRandomStream Noise(Cue*109+AudioVoice);
    for(int32 I=0;I<Count;++I)
    {
        const float T=I/24000.f, U=T/Duration;
        const float Envelope=FMath::Min(1.f,T/.008f)*FMath::Square(1-U);
        const float Frequency=Cue==5 ? (U<.33f ? 392.f : U<.66f ? 494.f : 587.f) : Cue==6 ? 115.f : Cue==3 ? 330.f : Cue==4 ? 80.f : Cue==2 ? 155.f : 230.f;
        const float Tone=FMath::Sin(2*PI*Frequency*T)+.25f*FMath::Sin(2*PI*Frequency*2.76f*T);
        const float NoiseMix=Cue==0 || Cue==3 || Cue==6 ? .50f : Cue==5 ? 0.f : .18f;
        PCM[I]=static_cast<int16>(FMath::Clamp((Tone*.55f+Noise.FRandRange(-1,1)*NoiseMix)*Envelope,-1.f,1.f)*10000);
    }
    Wave->QueueAudio(reinterpret_cast<const uint8*>(PCM.GetData()),PCM.Num()*sizeof(int16));
    AudioWaves[Index]=Wave; AudioVoices[Index]->SetSound(Wave); AudioVoices[Index]->SetVolumeMultiplier(.42f);
    AudioVoices[Index]->Play(); AudioTimers[Index]=Duration+.12f;
}

void ABGGameMode::Check(bool Condition,const FString& Message)
{
    if (!Condition) SmokeFailed=true;
    UE_LOG(LogTemp,Display,TEXT("BGTEST %s: %s"),Condition ? TEXT("PASS") : TEXT("FAIL"),*Message);
}

void ABGGameMode::TickSmokeTest(float Dt)
{
    const FName Screen=SettingsOpen ? FName(TEXT("19-Settings.png")) : Phase==EBGPhase::Briefing ? FName(TEXT("15-Briefing.png")) : Phase==EBGPhase::Route ? FName(TEXT("16-Routes.png")) : Phase==EBGPhase::Offering ? FName(TEXT("04-Offering.png")) : Phase==EBGPhase::Relic ? FName(TEXT("17-Relics.png")) : Phase==EBGPhase::Bargain ? FName(TEXT("18-Bargain.png")) : NAME_None;
    if(!Screen.IsNone() && !SmokeScreens.Contains(Screen))
    {
        SmokeScreens.Add(Screen);
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots")/Screen.ToString(),true,false);
        return;
    }
    SmokeTime+=Dt;
    if(SmokeTime<2) return;
    SmokeTime=0;
    auto Shot=[&](const FString& Name) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots")/Name,true,false); };
    auto Clear=[&]() { auto Targets=Enemies; for(auto& E:Targets) if(IsValid(E)) E->Hit(9999); HitStop=0; };
    auto* PC=Cast<ABGPlayerController>(GetWorld()->GetFirstPlayerController());
    auto Key=[&](FKey Value,EInputEvent Event) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Value,Event,Event==IE_Released ? 0 : 1)); };
    switch(SmokeStep++)
    {
    case 0:
        Check(Phase==EBGPhase::Harbor && Save && Save->SailorRecovery==0,TEXT("Fresh isolated harbor and injured sailor initialized"));
        SelectGod(1); Shot(TEXT("01-Harbor.png")); break;
    case 1:
        Key(EKeys::Gamepad_FaceButton_Bottom,IE_Pressed); break;
    case 2:
        Key(EKeys::Gamepad_FaceButton_Bottom,IE_Released);
        Check(Phase==EBGPhase::Briefing && Save->Expeditions==0,TEXT("Controller confirms sailing into crew briefing before counting a voyage"));
        Key(EKeys::Enter,IE_Pressed); break;
    case 3:
        Key(EKeys::Enter,IE_Released);
        Check(Phase==EBGPhase::Combat && Enemies.Num()==3,TEXT("Keyboard confirms departure and spawns the shore encounter"));
        { float H=Player()->Health; Check(Player()->CastPower(),TEXT("Zeus power casts")); Check(Player()->Health==H-8,TEXT("Zeus blood price applies")); Check(!Player()->CastPower(),TEXT("Power cooldown blocks repeated casts")); }
        Check(Player()->Dash(),TEXT("Dodge spends stamina"));
        { float H=Player()->Health; Player()->ReceiveHit(20); Check(Player()->Health==H,TEXT("Dodge protects against incoming damage")); }
        Shot(TEXT("03-Combat.png")); break;
    case 4:
        Clear(); break;
    case 5:
        Check(Phase==EBGPhase::Route && !IsPlaying(),TEXT("Cleared shore opens a paused route decision"));
        ChooseRoute(0); break;
    case 6:
        Check(Phase==EBGPhase::Offering && !RiskyRoute,TEXT("Safe route opens the first shrine"));
        ChooseOffering(0);
        Check(HasGod(EBGGod::Poseidon) && HasGod(EBGGod::Zeus) && Phase==EBGPhase::Relic,TEXT("Shrine stacks debts then offers a build relic")); break;
    case 7:
        ChooseRelic(0);
        Check(Relic==EBGRelic::Stormglass && Enemies.Num()==3 && Encounter==1,TEXT("Stormglass equipped on the shorter infirmary route"));
        HandleAction(TEXT("pause")); Check(!IsPlaying(),TEXT("Pause freezes combat")); Shot(TEXT("05-Pause.png")); break;
    case 8:
        HandleAction(TEXT("close")); Player()->Health=Player()->MaxHealth;
        if(Enemies.Num()==3)
        {
            auto* Main=Enemies[0].Get(); auto* Arc=Enemies[1].Get();
            Main->Health=Main->MaxHealth=500;
            Main->SetActorLocation(Player()->GetActorLocation()+FVector(500,0,0));
            Arc->SetActorLocation(Player()->GetActorLocation()+FVector(800,0,0));
            Enemies[2]->SetActorLocation(Player()->GetActorLocation()+FVector(1400,1000,0));
            SwitchGod(0); Player()->GodCooldowns[0]=0; Player()->RefreshPower(); float H=Player()->Health;
            Player()->CastPower();
            Check(Main->WetTimer==5 && Main->SlowTimer==3,TEXT("Tidebreak wets and slows the primary target"));
            Check(Player()->Health==H-8,TEXT("Zeus curse stays active while casting Tidebreak"));
            // Keep the primary within Stormcall and its neighbor outside it, inside arc range.
            Main->SetActorLocation(Player()->GetActorLocation()+FVector(650,0,0));
            SwitchGod(1); Player()->GodCooldowns[1]=0; Player()->RefreshPower();
            const float Before=Main->Health, ArcBefore=Arc->Health; Player()->CastPower();
            Check(FMath::IsNearlyEqual(Before-Main->Health,88.f) && Main->WetTimer==0,TEXT("Wet lightning consumes status for 26 bonus damage"));
            Check(FMath::IsNearlyEqual(ArcBefore-Arc->Health,18.f),TEXT("Stormglass arcs to a neighbor outside the power radius"));
            Player()->GodCooldowns[0]=3; SwitchGod(0); Check(Player()->PowerCooldown==3,TEXT("Switching retains independent cooldowns"));
        }
        Clear(); break;
    case 9:
        Check(Phase==EBGPhase::Bargain && Medicine==1,TEXT("Infirmary clear secures exactly one medicine flask"));
        ChooseBargain(1);
        Check(Medicine==1 && !TributeAccepted && SeaStrikesRemaining==3,TEXT("Refusal preserves medicine and schedules three warned sea strikes")); break;
    case 10:
        ChooseOffering(1);
        Check(Encounter==2 && RunDamageBonus==8 && Enemies.Num()==3,TEXT("Second shrine grants mortal bronze before the guardian"));
        Check(Enemies[2]->Armored,TEXT("Guardian guard includes a shielded sentinel"));
        Shot(TEXT("06-Guardian.png")); break;
    case 11:
        Player()->Invulnerable=10; Wrath=35; SeaStrikeDelay=.01f;
        break;
    case 12:
        Check(SeaStrikesRemaining==2 && Wrath>35,TEXT("Sea strike consumes one claim without resetting accumulated wrath"));
        if(Enemies.Num()>0) { auto* Boss=Enemies[0].Get(); Boss->Hit(260); Check(Boss->Enraged,TEXT("Talos changes phase below half health")); }
        Clear(); break;
    case 13:
        Check(AwaitingExtraction,TEXT("Guardian clear unlocks extraction"));
        Player()->SetActorLocation(FVector(-1100,-1100,100)); Interact();
        Check(Phase==EBGPhase::Victory && LastReward==109,TEXT("Safe route extraction banks full encounter rewards"));
        Check(Save->SailorRecovery==1 && Save->LastMedicine==1 && Save->HomewardProgress==1,TEXT("Preserved medicine advances Tomas and the homeward chart"));
        Shot(TEXT("07-Victory.png")); break;
    case 14:
        ReturnToHarbor(); HandleAction(TEXT("crew")); CrewSpeaker=1;
        Check(CrewDialogue().Contains(TEXT("recovering")),TEXT("Nera reacts to Tomas receiving one dose")); Shot(TEXT("13-Crew.png"));
        break;
    case 15:
        HandleAction(TEXT("close")); { int Before=Save->Obols;
        Check(Upgrade(false) && Save->Obols==Before-30 && Save->HullLevel==1,TEXT("Permanent upgrade spends banked currency")); }
        HandleAction(TEXT("settings")); break;
    case 16:
        Check(SettingsOpen && !IsPlaying(),TEXT("Input settings pause the world"));
        HandleAction(TEXT("bind_Power")); Key(EKeys::SpaceBar,IE_Pressed); Key(EKeys::SpaceBar,IE_Released);
        Check(PC->BoundKey(TEXT("Power"))==EKeys::SpaceBar && PC->BoundKey(TEXT("Dodge"))==EKeys::Q,TEXT("Rebinding swaps a conflicting action rather than duplicating it"));
        HandleAction(TEXT("bind_Attack")); Key(EKeys::Escape,IE_Pressed); Key(EKeys::Escape,IE_Released);
        Check(PC->Rebinding.IsNone() && PC->BoundKey(TEXT("Attack"))==EKeys::LeftMouseButton,TEXT("Escape cancels binding capture"));
        PC->BeginRebind(TEXT("Pause")); Check(PC->Rebinding.IsNone() && PC->BoundKey(TEXT("Pause"))==EKeys::Escape,TEXT("Reserved Escape cannot be swapped into another action"));
        HandleAction(TEXT("assist")); HandleAction(TEXT("motion")); HandleAction(TEXT("sound"));
        { auto* Loaded=Cast<UBGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot(),0)); Check(Loaded && Loaded->Assistance && Loaded->ReducedMotion && Loaded->Muted && Loaded->KeyBindings.Num()==2,TEXT("Accessibility and custom controls survive save/load")); }
        HandleAction(TEXT("close"));
        Phase=EBGPhase::Combat; Player()->Invulnerable=0; Player()->Health=100; Player()->ReceiveHit(20);
        Check(FMath::IsNearlyEqual(Player()->Health,87.f),TEXT("Assistance reduces incoming damage by 35 percent"));
        Impact(true); Check(HitStop==0,TEXT("Reduced motion suppresses hit-stop"));
        Phase=EBGPhase::Harbor; Save->Assistance=Save->ReducedMotion=Save->Muted=false; PC->ResetBindings();
        Feedback(2); Check(AudioWaves[(AudioVoice-1)%4]!=nullptr,TEXT("Runtime combat sound cue is generated without external assets"));
        SelectGod(0); StartExpedition(); Depart(); break;
    case 17:
        SmokeMoveStart=Player()->GetActorLocation(); Key(EKeys::W,IE_Pressed); break;
    case 18:
        Key(EKeys::W,IE_Released); Check(FVector::Dist2D(SmokeMoveStart,Player()->GetActorLocation())>300,TEXT("Keyboard movement follows camera-relative direction")); Clear(); break;
    case 19:
        Check(Phase==EBGPhase::Route,TEXT("Second voyage reaches the route decision"));
        ChooseRoute(1); ChooseOffering(0); break;
    case 20:
        ChooseRelic(1);
        { int Shields=0; for(auto& E:Enemies) if(E->Armored) ++Shields; Check(Enemies.Num()==5 && Shields==3 && Relic==EBGRelic::FerrymanThread,TEXT("Vault route adds three sentinels and equips the thread")); }
        if(Enemies.Num()>0)
        {
            auto* E=Enemies[0].Get(); E->Windup=.3f; E->AttackTarget=Player()->GetActorLocation();
            Player()->Health=60; Player()->Stamina=100; Player()->DashRemaining=0; Wrath=50;
            Player()->Dash(); Check(Player()->Health==68 && Wrath==38,TEXT("Timed dodge rewards thread healing and wrath relief"));
            Player()->DashRemaining=0; Player()->Stamina=100; Player()->Dash();
            Check(Player()->Health==68,TEXT("Thread cooldown prevents duplicate perfect-dodge rewards"));
        }
        Clear(); break;
    case 21:
        Check(Phase==EBGPhase::Bargain && Medicine==2,TEXT("Guarded vault secures two medicine flasks"));
        ChooseBargain(0); Check(Medicine==1 && TributeAccepted && SeaStrikesRemaining==0,TEXT("Vault tribute leaves one dose for the crew and prevents sea strikes"));
        ChooseOffering(2); Check(Player()->MaxHealth==135 && DebtCount()==2,TEXT("Mortal resolve increases health without another curse"));
        SwitchGod(0); Player()->GodCooldowns[0]=0; Player()->RefreshPower();
        if(Enemies.Num()>0)
        {
            auto* E=Enemies[0].Get(); E->Health=E->MaxHealth=500; E->SetActorLocation(Player()->GetActorLocation()+FVector(650,0,0));
            for(int I=1;I<Enemies.Num();++I) Enemies[I]->SetActorLocation(Player()->GetActorLocation()+FVector(1400,1000,0));
            float W=Wrath; Player()->CastPower();
            Check(FMath::IsNearlyEqual(E->Health,438.f),TEXT("Tribute increases Tidebreak range and damage"));
            Check(FMath::IsNearlyEqual(Wrath,FMath::Min(100.f,W+23)),TEXT("Poseidon retains its increased wrath cost"));
        }
        Clear(); break;
    case 22:
        Player()->SetActorLocation(FVector(-1100,-1100,100)); Interact();
        Check(Phase==EBGPhase::Victory && LastReward==125,TEXT("Risky route pays for its extra enemies"));
        Check(Save->SailorRecovery==2 && Save->MedicineDelivered==2 && Save->LastTribute && Save->LastRiskyRoute,TEXT("A second delivered dose returns Tomas to duty and remembers the bargain"));
        Shot(TEXT("20-Vault-Return.png")); break;
    case 23:
        ReturnToHarbor(); HandleAction(TEXT("crew")); CrewSpeaker=1;
        Check(CrewDialogue().Contains(TEXT("deck")),TEXT("Recovered sailor changes the crew's return dialogue")); Shot(TEXT("21-Tomas-Recovered.png"));
        { auto* Loaded=Cast<UBGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot(),0)); Check(Loaded && Loaded->SailorRecovery==2 && Loaded->MedicineDelivered==2 && Loaded->LastTribute,TEXT("Crew consequence survives SaveGame round trip")); }
        break;
    case 24:
        HandleAction(TEXT("close")); Save->SailorRecovery=0; Save->MedicineDelivered=0; SelectGod(2); StartExpedition(); Depart();
        Check(Player()->MaxStamina==75 && Player()->MaxHealth==115,TEXT("Hades burden and permanent hull upgrade apply")); break;
    case 25:
        if(Enemies.Num()>0)
        {
            for(int I=0;I<Enemies.Num();++I) Enemies[I]->SetActorLocation(Player()->GetActorLocation()+FVector(I==0 ? 140 : 1400,I==0 ? 0 : 1000,0));
            Player()->Health=40; Player()->CastPower(); Check(Player()->Health==52,TEXT("Soul Harvest heals only for nearby enemies"));
        }
        Clear(); break;
    case 26:
        ChooseRoute(0); ChooseOffering(0); ChooseRelic(2);
        Check(Relic==EBGRelic::Oathstone && Enemies.Num()==3,TEXT("Oathstone can be chosen on the safe route"));
        if(Enemies.Num()>0)
        {
            auto* E=Enemies[0].Get(); E->Configure(false,false,true); E->Health=E->MaxHealth=500;
            E->SetActorLocation(Player()->GetActorLocation()+FVector(140,0,0));
            for(int I=1;I<Enemies.Num();++I) Enemies[I]->SetActorLocation(Player()->GetActorLocation()+FVector(1400,1000,0));
            float H=E->Health; E->Hit(40,FVector::ZeroVector,true); Check(FMath::IsNearlyEqual(H-E->Health,14.f),TEXT("Closed sentinel shield reduces ordinary melee damage"));
            E->Windup=.7f; E->ShieldOpen=1; Player()->Facing=FVector(1,0,0); Player()->Stamina=30; Player()->ComboStep=0;
            for(int I=0;I<3;++I)
            {
                HitStop=0; Player()->AttackCooldown=0; Player()->AttackWindup=0;
                float Before=E->Health; Player()->Attack();
                Check(E->Health==Before && Player()->AttackWindup>0,TEXT("Melee damage waits for its readable windup"));
                Player()->AttackWindup=0; Player()->ResolveAttack();
            }
            Check(E->Windup==0 && E->StaggerTimer>0 && Player()->Stamina==54,TEXT("Oathstone finisher interrupts and restores stamina"));
        }
        Clear(); break;
    case 27:
        Check(Medicine==1 && Phase==EBGPhase::Bargain,TEXT("Third voyage's infirmary supplies are secured"));
        ChooseBargain(0); ChooseOffering(0);
        Check(DebtCount()==3 && HasGod(EBGGod::Zeus) && Player()->MaxStamina==75,TEXT("Three borrowed gods retain every curse"));
        SwitchGod(0); Player()->GodCooldowns[0]=0; Player()->RefreshPower();
        { float H=Player()->Health; Player()->CastPower(); Check(Player()->Health==FMath::Max(1.f,H-8),TEXT("Zeus price follows the player across all three masks")); }
        Clear(); break;
    case 28:
        Player()->SetActorLocation(FVector(-1100,-1100,100)); Interact();
        Check(Phase==EBGPhase::Victory && Save->LastMedicine==0 && Save->SailorRecovery==0,TEXT("Paying the safe route's only flask leaves Tomas untreated"));
        Check(Save->HomewardProgress==3 && Save->LastConsequence.Contains(TEXT("fever")),TEXT("Chart can complete while the crew's need remains unmet"));
        Shot(TEXT("22-Power-Over-Promise.png")); break;
    case 29:
        ReturnToHarbor(); HandleAction(TEXT("crew")); CrewSpeaker=1;
        Check(CrewDialogue().Contains(TEXT("no medicine")),TEXT("Nera remembers choosing power over Tomas")); Shot(TEXT("23-Nera-Consequence.png")); break;
    case 30:
        CrewSpeaker=0; Check(CrewDialogue().Contains(TEXT("course home")),TEXT("Three genuine recoveries resolve the homeward chart")); Shot(TEXT("14-Homeward.png"));
        break;
    case 31:
        HandleAction(TEXT("close")); SelectGod(0); StartExpedition(); Depart(); break;
    case 32:
        Clear(); break;
    case 33:
        ChooseRoute(1); ChooseOffering(2); ChooseRelic(2); Clear(); break;
    case 34:
        Check(Phase==EBGPhase::Bargain && Medicine==2,TEXT("Failed-return scenario reaches the vault bargain"));
        HandleAction(TEXT("abandon"));
        Check(Phase==EBGPhase::Defeat && LastReward==16 && Save->LastMedicine==0 && Save->SailorRecovery==0,TEXT("Ending at a decision banks a quarter but loses carried medicine"));
        Check(Save->HomewardProgress==3 && Save->VoyageJournal.Num()==4,TEXT("Failure records a voyage without changing the chart")); Shot(TEXT("08-Defeat.png")); break;
    case 35:
        ReturnToHarbor(); HandleAction(TEXT("settings")); Key(EKeys::Gamepad_DPad_Down,IE_Pressed); break;
    case 36:
        Key(EKeys::Gamepad_DPad_Down,IE_Released);
        Check(Cast<ABGHUD>(PC->GetHUD())->FocusAction!=TEXT("close"),TEXT("Controller D-pad navigates settings without mouse input"));
        HandleAction(TEXT("close")); HandleAction(TEXT("journal")); Check(!IsPlaying(),TEXT("Journal pauses the voyage")); Shot(TEXT("02-Journal.png")); break;
    case 37:
        HandleAction(TEXT("close")); HandleAction(TEXT("help")); Shot(TEXT("11-Controls.png"));
        { auto* Loaded=Cast<UBGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot(),0)); Check(Loaded && Loaded->HomewardProgress==3 && Loaded->VoyageJournal.Num()==4 && Loaded->SailorRecovery==0,TEXT("Story and crew state survive final save/load")); } break;
    default:
        FFileHelper::SaveStringToFile(SmokeFailed ? TEXT("FAIL") : TEXT("PASS"),*(FPaths::ProjectSavedDir()/TEXT("SmokeTestResult.txt")));
        UE_LOG(LogTemp,Display,TEXT("BGTEST COMPLETE: %s"),SmokeFailed ? TEXT("FAIL") : TEXT("PASS"));
        FPlatformMisc::RequestExitWithStatus(false,SmokeFailed ? 1 : 0); break;
    }
}
