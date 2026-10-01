#include "BGPlayerController.h"
#include "BGGameMode.h"
#include "BGCharacter.h"
#include "BGHUD.h"
#include "BGSaveGame.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"

ABGPlayerController::ABGPlayerController()
{
    bShowMouseCursor=true; bEnableClickEvents=true; bEnableMouseOverEvents=true;
    DefaultMouseCursor=EMouseCursor::Crosshairs;
}
TArray<FName> ABGPlayerController::BindingActions()
{
    return {TEXT("Forward"),TEXT("Back"),TEXT("Left"),TEXT("Right"),TEXT("Attack"),TEXT("Dodge"),TEXT("Power"),TEXT("Cycle"),TEXT("Interact"),TEXT("Journal"),TEXT("Crew"),TEXT("Help"),TEXT("Pause"),TEXT("Confirm")};
}
FKey ABGPlayerController::DefaultKey(FName Action)
{
    const TArray<FName> Actions=BindingActions();
    const FKey Keys[]={EKeys::W,EKeys::S,EKeys::A,EKeys::D,EKeys::LeftMouseButton,EKeys::SpaceBar,EKeys::Q,EKeys::R,EKeys::E,EKeys::Tab,EKeys::C,EKeys::F1,EKeys::Escape,EKeys::Enter};
    const int32 Index=Actions.IndexOfByKey(Action);
    return Index>=0 ? Keys[Index] : EKeys::Invalid;
}
FKey ABGPlayerController::BoundKey(FName Action) const
{
    const auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    if(G && G->Save) if(const auto* Key=G->Save->KeyBindings.Find(Action)) { FKey Value(*Key); if(Value.IsValid()) return Value; }
    return DefaultKey(Action);
}
FString ABGPlayerController::KeyLabel(FName Action) const { return BoundKey(Action).GetDisplayName().ToString().ToUpper(); }
void ABGPlayerController::BeginRebind(FName Action) { if(Action!=TEXT("Pause") && BindingActions().Contains(Action)) Rebinding=Action; }
void ABGPlayerController::ResetBindings() { Rebinding=NAME_None; if(auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>()) { G->Save->KeyBindings.Empty(); G->SaveProgress(); } }
bool ABGPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
    if(Params.Event==IE_Pressed)
    {
        UsingGamepad=Params.Key.IsGamepadKey();
        if(!Rebinding.IsNone())
        {
            if(Params.Key==EKeys::Escape || Params.Key==EKeys::Gamepad_FaceButton_Right) { Rebinding=NAME_None; return true; }
            if(Params.Key.IsGamepadKey() || Params.Key==EKeys::One || Params.Key==EKeys::Two || Params.Key==EKeys::Three || Params.Key.IsAxis1D() || Params.Key.IsAxis2D()) return true;
            if(auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>(); G && G->SettingsOpen)
            {
                const FKey Previous=BoundKey(Rebinding);
                for(FName Action:BindingActions()) if(Action!=Rebinding && BoundKey(Action)==Params.Key) G->Save->KeyBindings.Add(Action,Previous.GetFName());
                G->Save->KeyBindings.Add(Rebinding,Params.Key.GetFName()); G->SaveProgress(); Rebinding=NAME_None;
            }
            return true;
        }
    }
    return Super::InputKey(Params);
}
void ABGPlayerController::BeginPlay()
{
    Super::BeginPlay(); FInputModeGameAndUI Mode; Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); SetInputMode(Mode);
}
void ABGPlayerController::PlayerTick(float Dt)
{
    Super::PlayerTick(Dt);
    auto* G=GetWorld()->GetAuthGameMode<ABGGameMode>(); auto* P=Cast<ABGCharacter>(GetPawn());
    if(!G || !P || !Rebinding.IsNone()) return;
    auto Press=[&](FName Action) { return WasInputKeyJustPressed(BoundKey(Action)); };
    if(Press(TEXT("Pause")) || WasInputKeyJustPressed(EKeys::Escape) || WasInputKeyJustPressed(EKeys::Gamepad_Special_Right)) G->HandleAction(TEXT("pause"));
    if(Press(TEXT("Journal")) || WasInputKeyJustPressed(EKeys::Gamepad_Special_Left)) G->HandleAction(TEXT("journal"));
    if(Press(TEXT("Help"))) G->HandleAction(TEXT("help"));
    if(Press(TEXT("Crew"))) G->HandleAction(TEXT("crew"));
    const bool Menu=G->Phase!=EBGPhase::Combat || G->Paused || G->CrewOpen || G->JournalOpen || G->HelpOpen || G->SettingsOpen;
    if(Menu)
    {
        if(auto* HUD=Cast<ABGHUD>(GetHUD()))
        {
            if(WasInputKeyJustPressed(EKeys::Gamepad_DPad_Right) || WasInputKeyJustPressed(EKeys::Gamepad_DPad_Down)) HUD->MoveFocus(1);
            if(WasInputKeyJustPressed(EKeys::Gamepad_DPad_Left) || WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up)) HUD->MoveFocus(-1);
            if(WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom)) HUD->ActivateFocus();
        }
        if(WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right)) G->HandleAction(TEXT("pause"));
    }
    if(WasInputKeyJustPressed(EKeys::One)) G->HandleAction(TEXT("choice0"));
    if(WasInputKeyJustPressed(EKeys::Two)) G->HandleAction(TEXT("choice1"));
    if(WasInputKeyJustPressed(EKeys::Three)) G->HandleAction(TEXT("choice2"));
    if(Press(TEXT("Confirm")) && !G->Paused && !G->JournalOpen && !G->HelpOpen && !G->CrewOpen && !G->SettingsOpen)
    {
        if(G->Phase==EBGPhase::Harbor) G->HandleAction(TEXT("sail"));
        else if(G->Phase==EBGPhase::Briefing) G->HandleAction(TEXT("depart"));
        else if(G->Phase==EBGPhase::Victory || G->Phase==EBGPhase::Defeat) G->HandleAction(TEXT("harbor"));
    }
    if(!G->IsPlaying()) { P->MoveDirection=FVector::ZeroVector; return; }
    float X=(IsInputKeyDown(BoundKey(TEXT("Forward"))) ? 1.f : 0.f)-(IsInputKeyDown(BoundKey(TEXT("Back"))) ? 1.f : 0.f);
    float Y=(IsInputKeyDown(BoundKey(TEXT("Right"))) ? 1.f : 0.f)-(IsInputKeyDown(BoundKey(TEXT("Left"))) ? 1.f : 0.f);
    const float LX=GetInputAnalogKeyState(EKeys::Gamepad_LeftX), LY=GetInputAnalogKeyState(EKeys::Gamepad_LeftY);
    if(FVector2D(LX,LY).Size()>.2f) { X=LY; Y=LX; UsingGamepad=true; }
    P->MoveDirection=(FVector(.7071,-.7071,0)*X+FVector(.7071,.7071,0)*Y).GetClampedToMaxSize(1);
    const float RX=GetInputAnalogKeyState(EKeys::Gamepad_RightX), RY=GetInputAnalogKeyState(EKeys::Gamepad_RightY);
    if(FVector2D(RX,RY).Size()>.2f) { P->Facing=(FVector(.7071,-.7071,0)*RY+FVector(.7071,.7071,0)*RX).GetSafeNormal2D(); UsingGamepad=true; }
    else if(!UsingGamepad)
    {
        FVector Origin,Direction;
        if(DeprojectMousePositionToWorld(Origin,Direction) && FMath::Abs(Direction.Z)>.001f)
        {
            float T=(P->GetActorLocation().Z-Origin.Z)/Direction.Z;
            if(T>0) P->Facing=(Origin+Direction*T-P->GetActorLocation()).GetSafeNormal2D();
        }
    }
    if(IsInputKeyDown(BoundKey(TEXT("Attack"))) || IsInputKeyDown(EKeys::Gamepad_FaceButton_Left)) P->Attack();
    if(Press(TEXT("Dodge")) || WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom)) P->Dash();
    if(Press(TEXT("Power")) || WasInputKeyJustPressed(EKeys::Gamepad_RightTrigger)) P->CastPower();
    if(Press(TEXT("Cycle")) || WasInputKeyJustPressed(EKeys::Gamepad_RightShoulder)) G->CycleGod();
    if(WasInputKeyJustPressed(EKeys::Gamepad_LeftShoulder)) for(int I=1;I<=3;++I) { const int Index=(static_cast<int>(G->God)+3-I)%3; if(G->HasGod(static_cast<EBGGod>(Index))) { G->SwitchGod(Index); break; } }
    if(Press(TEXT("Interact")) || WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Top)) G->Interact();
}
