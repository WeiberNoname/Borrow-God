#include "BGHUD.h"
#include "BGGameMode.h"
#include "BGCharacter.h"
#include "BGEnemy.h"
#include "BGSaveGame.h"
#include "BGPlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"

ABGHUD::ABGHUD()
{
    static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
    Regular=Font.Object; Bold=Font.Object;
}
void ABGHUD::Rect(float X,float Y,float W,float H,FLinearColor Color)
{
    const bool Surface=(Color==Ink || Color==Panel) && W>100 && H>45 && W<1440;
    if(Surface) DrawRect(FLinearColor(0,0,0,.22),OX+(X+3)*S,OY+(Y+5)*S,W*S,H*S);
    DrawRect(Color,OX+X*S,OY+Y*S,W*S,H*S);
    if(Surface)
    {
        Line(X,Y,X+W,Y,Gold.CopyWithNewOpacity(.22));
        Line(X,Y+H,X+W,Y+H,Gold.CopyWithNewOpacity(.10));
        Line(X,Y,X+9,Y,Gold.CopyWithNewOpacity(.65),2);
        Line(X+W-9,Y+H,X+W,Y+H,Gold.CopyWithNewOpacity(.65),2);
    }
    if(Color==Ink && W==1440 && H==824)
    {
        // Quiet engraved compass lines give menus the same nautical language as the island.
        for(int32 Ring=0;Ring<4;++Ring) for(int32 I=0;I<96;++I)
        {
            float A=I*2*PI/96, B=(I+1)*2*PI/96, R=270+Ring*72;
            Line(1210+FMath::Cos(A)*R,493+FMath::Sin(A)*R,1210+FMath::Cos(B)*R,493+FMath::Sin(B)*R,Gold.CopyWithNewOpacity(.035));
        }
        for(int32 I=0;I<24;++I)
        {
            float A=I*2*PI/24;
            Line(1210+FMath::Cos(A)*248,493+FMath::Sin(A)*248,1210+FMath::Cos(A)*263,493+FMath::Sin(A)*263,Gold.CopyWithNewOpacity(.09));
        }
        Line(48,887,1392,887,Gold.CopyWithNewOpacity(.12));
        Diamond(720,887,4,Gold.CopyWithNewOpacity(.5));
    }
}
void ABGHUD::Line(float X,float Y,float X2,float Y2,FLinearColor Color,float Width)
{
    // Canvas line primitives are opaque. Mix decorative strokes into the ink palette explicitly.
    const FLinearColor Stroke=FMath::Lerp(Ink,Color,FMath::Clamp(Color.A,0.f,1.f));
    DrawLine(OX+X*S,OY+Y*S,OX+X2*S,OY+Y2*S,Stroke.CopyWithNewOpacity(1),Width*S);
}
void ABGHUD::Border(float X,float Y,float W,float H,FLinearColor Color)
{ Line(X,Y,X+W,Y,Color); Line(X+W,Y,X+W,Y+H,Color); Line(X+W,Y+H,X,Y+H,Color); Line(X,Y+H,X,Y,Color); }
void ABGHUD::Text(const FString& Value,float X,float Y,float Size,FLinearColor Color,bool Strong,bool Center)
{
    UFont* Font=Strong ? Bold.Get() : Regular.Get();
    if (!Font) Font=GEngine->GetMediumFont();
    float Native=FMath::Max(1.f,static_cast<float>(Font->GetMaxCharHeight()));
    Canvas->K2_DrawText(Font,Value,FVector2D(OX+X*S,OY+Y*S),FVector2D(Size/Native*S),Color,0,FLinearColor::Black,FVector2D::ZeroVector,Center,false,false);
}
void ABGHUD::Wrap(const FString& Value,float X,float Y,float W,float Size,FLinearColor Color,float Spacing)
{
    TArray<FString> Words; Value.ParseIntoArray(Words,TEXT(" "),true);
    FString Row;
    const float Scale=Size/FMath::Max(1.f,static_cast<float>(Regular->GetMaxCharHeight()));
    for (const auto& Word:Words)
    {
        FString Candidate=Row.IsEmpty() ? Word : Row+TEXT(" ")+Word;
        float TW,TH; GetTextSize(Candidate,TW,TH,Regular,Scale);
        if (TW>W && !Row.IsEmpty()) { Text(Row,X,Y,Size,Color); Y+=Size*Spacing; Row=Word; }
        else Row=Candidate;
    }
    if (!Row.IsEmpty()) Text(Row,X,Y,Size,Color);
}
void ABGHUD::Diamond(float X,float Y,float R,FLinearColor Color)
{ Line(X,Y-R,X+R,Y,Color); Line(X+R,Y,X,Y+R,Color); Line(X,Y+R,X-R,Y,Color); Line(X-R,Y,X,Y-R,Color); }
void ABGHUD::Icon(int32 Type,float X,float Y,float R,FLinearColor Color)
{
    Diamond(X,Y,R,Color.CopyWithNewOpacity(.35));
    if (Type==0)
    {
        Line(X,Y-R*.7,X,Y+R*.7,Color,2); Line(X-R*.42,Y-R*.4,X-R*.42,Y+R*.08,Color,2);
        Line(X+R*.42,Y-R*.4,X+R*.42,Y+R*.08,Color,2); Line(X-R*.42,Y+R*.08,X+R*.42,Y+R*.08,Color,2);
        Line(X-R*.13,Y-R*.48,X,Y-R*.7,Color,2); Line(X,Y-R*.7,X+R*.13,Y-R*.48,Color,2);
    }
    else if (Type==1)
    {
        Line(X+R*.22,Y-R*.65,X-R*.3,Y+R*.05,Color,3); Line(X-R*.3,Y+R*.05,X+R*.3,Y-R*.05,Color,3); Line(X+R*.3,Y-R*.05,X-R*.22,Y+R*.65,Color,3);
    }
    else
    {
        Diamond(X,Y-R*.1,R*.36,Color); Line(X-R*.25,Y+R*.35,X+R*.25,Y+R*.35,Color,2);
        Line(X-R*.14,Y+R*.16,X-R*.14,Y+R*.55,Color,2); Line(X+R*.14,Y+R*.16,X+R*.14,Y+R*.55,Color,2);
    }
}
void ABGHUD::Button(FName ID,const FString& Label,float X,float Y,float W,float H,bool Primary,bool Enabled)
{
    const bool Hover=Enabled && Mouse.X>=X && Mouse.X<=X+W && Mouse.Y>=Y && Mouse.Y<=Y+H;
    FLinearColor Fill=Primary ? Gold : Panel;
    if (Hover) Fill=Primary ? FLinearColor(.98,.83,.54) : FLinearColor(.10,.16,.18);
    if (!Enabled) Fill=FLinearColor(.045,.055,.06);
    Rect(X,Y,W,H,Fill);
    Border(X,Y,W,H,Enabled ? (Primary ? Gold : FLinearColor(.23,.32,.33)) : FLinearColor(.11,.15,.16));
    Text(Label,X+W/2,Y+(H-16)/2,16,Enabled ? (Primary ? Ink : White) : Muted,true,true);
    if (Enabled) Focusable(ID,X,Y,W,H);
}
void ABGHUD::Focusable(FName ID,float X,float Y,float W,float H)
{
    MenuActions.AddUnique(ID);
    AddHitBox(FVector2D(OX+X*S,OY+Y*S),FVector2D(W*S,H*S),ID,true,10);
    auto* PC=Cast<ABGPlayerController>(GetOwningPlayerController());
    if(PC && PC->UsingGamepad && FocusAction==ID) { Border(X-4,Y-4,W+8,H+8,Teal); Diamond(X+12,Y+H/2,4,Teal); }
}
void ABGHUD::MoveFocus(int32 Direction)
{
    if(MenuActions.IsEmpty()) return;
    int32 Index=MenuActions.IndexOfByKey(FocusAction);
    if(Index<0) Index=0;
    FocusAction=MenuActions[(Index+MenuActions.Num()+Direction)%MenuActions.Num()];
}
void ABGHUD::ActivateFocus() { if(G && MenuActions.Contains(FocusAction)) G->HandleAction(FocusAction); }
void ABGHUD::Bar(float X,float Y,float W,float H,float Fraction,FLinearColor Color)
{ Rect(X,Y,W,H,FLinearColor(.09,.13,.14,.9)); Rect(X,Y,W*FMath::Clamp(Fraction,0.f,1.f),H,Color); }

void ABGHUD::Header(const FString& Section)
{
    auto* PC=Cast<ABGPlayerController>(GetOwningPlayerController());
    auto Shortcut=[&](FName Action) { FString Key=PC->KeyLabel(Action); return Key.Len()>7 ? FString() : TEXT(" [")+Key+TEXT("]"); };
    Rect(0,0,1440,76,Ink);
    Diamond(44,38,17,Gold); Text(TEXT("B"),44,25,24,Gold,true,true);
    Text(TEXT("BORROWED GODS"),77,25,23,White,true);
    Line(301,26,301,50,Muted.CopyWithNewOpacity(.35)); Text(Section,326,30,13,Gold);
    Button(TEXT("crew"),TEXT("THE CREW")+Shortcut(TEXT("Crew")),810,21,146,36);
    Button(TEXT("journal"),TEXT("JOURNAL")+Shortcut(TEXT("Journal")),968,21,164,36);
    Button(TEXT("help"),TEXT("CONTROLS")+Shortcut(TEXT("Help")),1144,21,160,36);
    Button(TEXT("pause"),TEXT("ESC"),1316,21,76,36);
    Line(0,75,1440,75,Gold.CopyWithNewOpacity(.3));
}

void ABGHUD::Harbor()
{
    Rect(0,76,1440,824,Ink);
    Header(TEXT("THE ARGO  /  BETWEEN VOYAGES"));
    Text(TEXT("A MORTAL'S VOYAGE"),48,111,13,Gold);
    Text(TEXT("Home is beyond"),45,137,46,White,true);
    Text(TEXT("the gods."),45,186,46,White,true);
    Wrap(TEXT("Steal their power. Bear their curse. Find your way home."),48,252,620,18,Muted);
    Rect(1090,111,302,93,Panel); Diamond(1120,142,10,Gold);
    Text(FString::Printf(TEXT("%03d"),G->Save->Obols),1144,122,29,White,true);
    Text(TEXT("OBOLS BANKED"),1145,163,11,Muted);
    Text(FString::Printf(TEXT("%d voyages   /   %d relics recovered"),G->Save->Expeditions,G->Save->Victories),1092,216,13,Muted);
    Text(FString::Printf(TEXT("HOMEWARD CHART  /  %d OF 3 FRAGMENTS"),G->Save->HomewardProgress),1092,243,12,Gold);

    // Stylized navigation chart: visual UI, no generated art dependency.
    const float MX=48, MY=304, MW=650, MH=350;
    Rect(MX,MY,MW,MH,FLinearColor(.038,.075,.09)); Border(MX,MY,MW,MH,FLinearColor(.14,.23,.25));
    for (int I=1;I<10;++I) Line(MX+I*65,MY,MX+I*65,MY+MH,FLinearColor(.07,.12,.14));
    for (int I=1;I<6;++I) Line(MX,MY+I*58,MX+MW,MY+I*58,FLinearColor(.07,.12,.14));
    Text(TEXT("THE SUNDERED AEGEAN"),70,326,13,Muted);
    Text(TEXT("N"),650,331,12,Gold,true,true); Line(650,355,650,383,Gold); Diamond(650,353,5,Gold);
    // Hand-drawn island contours, a dotted voyage path and chart landmarks.
    const FVector2D Outline[]={{351,410},{388,394},{429,412},{475,406},{517,445},{499,474},{515,514},{477,548},{434,537},{405,557},{366,527},{374,495},{345,460},{351,410}};
    for(int Ring=0;Ring<3;++Ring) for(int I=0;I<13;++I)
    {
        FVector2D A=(Outline[I]-FVector2D(430,475))*(1+Ring*.12f)+FVector2D(430,475);
        FVector2D B=(Outline[I+1]-FVector2D(430,475))*(1+Ring*.12f)+FVector2D(430,475);
        Line(A.X,A.Y,B.X,B.Y,Ring==0 ? Teal.CopyWithNewOpacity(.8) : Teal.CopyWithNewOpacity(.18));
    }
    for(int I=0;I<20;++I) { float T=I/20.f; Diamond(145+T*243,560-T*70+FMath::Sin(T*PI)*20,1.5,Gold); }
    Icon(0,430,465,34,Gold); Diamond(145,565,12,White);
    Text(TEXT("THE ARGO"),111,591,12,White); Text(TEXT("01"),474,435,12,Teal);
    Text(TEXT("DROWNED SANCTUARY"),355,592,14,White,true);
    Text(TEXT("POSEIDON'S DOMAIN"),375,616,10,Muted);
    Text(TEXT("35.4 N  /  24.8 E"),71,626,10,Muted);

    Text(TEXT("01  /  CHOOSE YOUR STOLEN POWER"),744,276,13,Gold);
    const FString Names[]={TEXT("POSEIDON"),TEXT("ZEUS"),TEXT("HADES")};
    const FString Titles[]={TEXT("Tidebreak"),TEXT("Stormcall"),TEXT("Soul Harvest")};
    const FString Desc[]={TEXT("Push back and slow nearby enemies."),TEXT("Strike every enemy in a wide radius."),TEXT("Drain nearby enemies to heal yourself.")};
    const FString Curse[]={TEXT("CURSE  /  Wrath rises faster"),TEXT("CURSE  /  Casting costs 8 health"),TEXT("CURSE  /  Maximum stamina: 75")};
    for(int I=0;I<3;++I)
    {
        float Y=304+I*111; bool Selected=static_cast<int32>(G->God)==I;
        Rect(744,Y,648,98,Selected ? FLinearColor(.075,.13,.145) : Panel);
        Border(744,Y,648,98,Selected ? Gold : FLinearColor(.12,.19,.20));
        if(Selected) Rect(744,Y,3,98,Gold);
        Icon(I,789,Y+46,27,BGRules::GodColor(static_cast<EBGGod>(I)));
        Text(Names[I]+TEXT("  /  ")+Titles[I],834,Y+13,18,White,true);
        Text(Desc[I],834,Y+42,14,Muted);
        Text(Curse[I],834,Y+67,12,Selected ? Gold : Muted);
        Text(Selected ? TEXT("EQUIPPED") : FString::Printf(TEXT("[%d]"),I+1),1369,Y+15,10,Selected ? Gold : Muted,false,true);
        Focusable(FName(*FString::Printf(TEXT("choice%d"),I)),744,Y,648,98);
    }

    Text(TEXT("02  /  PREPARE YOUR VOYAGE"),48,680,13,Gold);
    const int HullCost=BGRules::UpgradeCost(G->Save->HullLevel), BladeCost=BGRules::UpgradeCost(G->Save->BladeLevel);
    Rect(48,709,315,103,Panel); Text(TEXT("REINFORCED OATH"),65,725,14,White,true);
    Text(FString::Printf(TEXT("+15 health / level %d of 5"),G->Save->HullLevel),65,752,12,Muted);
    Button(TEXT("hull"),G->Save->HullLevel>=5 ? TEXT("COMPLETE") : FString::Printf(TEXT("UPGRADE  /  %d OBOLS"),HullCost),65,777,280,25,false,G->Save->HullLevel<5 && G->Save->Obols>=HullCost);
    Rect(380,709,318,103,Panel); Text(TEXT("TEMPERED BRONZE"),397,725,14,White,true);
    Text(FString::Printf(TEXT("+4 melee damage / level %d of 5"),G->Save->BladeLevel),397,752,12,Muted);
    Button(TEXT("blade"),G->Save->BladeLevel>=5 ? TEXT("COMPLETE") : FString::Printf(TEXT("UPGRADE  /  %d OBOLS"),BladeCost),397,777,284,25,false,G->Save->BladeLevel<5 && G->Save->Obols>=BladeCost);
    Text(TEXT("POWER IS A CHOICE. SO IS REFUSING IT."),744,676,13,Muted);
    Wrap(TEXT("Nera needs medicine for Tomas. A safe path yields one flask; a guarded vault holds two. Poseidon will demand his share."),744,702,620,15,Muted);
    Button(TEXT("sail"),TEXT("SET SAIL    /    ENTER"),744,765,648,55,true);
    Line(48,844,1392,844,FLinearColor(.15,.22,.23));
    Text(TEXT("CHAPTER I   -   A DEBT TO THE SEA"),48,862,11,Muted);
    Text(TEXT("THE SEA REMEMBERS EVERY DEBT"),1150,862,10,Muted);
}

void ABGHUD::Combat()
{
    ABGCharacter* P=G->Player();
    Header(TEXT("CHAPTER I  /  DROWNED SANCTUARY"));
    Rect(32,104,353,115,Ink); Rect(32,104,3,115,Gold);
    Text(TEXT("THE OATHBREAKER"),52,120,14,White,true);
    Text(FString::Printf(TEXT("%d / %d"),FMath::CeilToInt(P->Health),FMath::RoundToInt(P->MaxHealth)),358,120,13,White,false,true);
    Bar(52,151,310,9,P->Health/P->MaxHealth,FLinearColor(.7,.25,.2));
    Text(TEXT("VITALITY"),52,166,10,Muted);
    Bar(52,190,310,5,P->Stamina/P->MaxStamina,Teal);
    Text(TEXT("DODGE"),52,200,9,Muted);
    Rect(32,231,353,50,Ink); Text(BGRules::RelicName(G->Relic),52,242,13,Gold);
    Text(P->ComboStep==2 ? TEXT("NEXT STRIKE: OATHBREAK") : TEXT("HOLD ATTACK / THREE-HIT COMBO"),52,263,10,Muted);

    Rect(1027,104,381,113,Ink);
    Text(TEXT("CURRENT OBJECTIVE"),1047,121,11,Gold);
    Wrap(G->Objective(),1047,146,330,16,White);
    Text(FString::Printf(TEXT("%02d obols carried  /  %02d:%02d"),G->CarriedObols,static_cast<int>(G->RunTime)/60,static_cast<int>(G->RunTime)%60),1047,190,12,Muted);
    Rect(1027,230,381,81,Ink);
    Text(FString::Printf(TEXT("MEDICINE / %d FLASKS"),G->Medicine),1047,242,13,Teal);
    Text(G->TributeAccepted ? TEXT("TRIBUTE PAID / +20 POWER DAMAGE") : G->Encounter==2 ? FString::Printf(TEXT("TRIBUTE REFUSED / %d SEA STRIKES LEFT"),G->SeaStrikesRemaining) : TEXT("NERA'S REQUEST / BRING TOMAS A DOSE"),1047,269,11,Gold);
    Text(G->RiskyRoute ? TEXT("ROUTE: GUARDED VAULT") : TEXT("ROUTE: HEALER'S PATH"),1047,292,10,Muted);
    Rect(520,104,340,67,Ink);
    for(int I=0;I<3;++I)
    {
        float X=558+I*122;
        if(I<2) Line(X+9,121,X+112,121,FLinearColor(.28,.34,.34));
        Diamond(X,121,8,I<=G->Encounter ? Gold : Muted);
        Text(I==0 ? TEXT("SHORE") : I==1 ? (G->RiskyRoute ? TEXT("VAULT") : TEXT("INFIRMARY")) : TEXT("SANCTUM"),X,141,10,I==G->Encounter ? White : Muted,false,true);
    }

    // Enemy bars and attack tells are projected into screen space.
    for(auto& E:G->Enemies)
    {
        if(!IsValid(E) || E->Dead) continue;
        FVector P2=Project(E->GetActorLocation()+FVector(0,0,E->Boss ? 340 : 180));
        if(P2.Z<=0) continue;
        float X=(P2.X-OX)/S,Y=(P2.Y-OY)/S;
        if(X<405 || X>1010 || Y<240 || Y>688)
        {
            const float EdgeX=FMath::Clamp(X,405.f,1010.f), EdgeY=FMath::Clamp(Y,269.f,690.f);
            Diamond(EdgeX,EdgeY,7,Red); Text(E->Boss ? TEXT("GUARDIAN") : TEXT("ENEMY"),EdgeX,EdgeY+12,9,Red,false,true);
        }
        if(!E->Boss && Y>90 && Y<700 && X>20 && X<1420)
        {
            Bar(X-33,Y,66,4,E->Health/E->MaxHealth,Red);
            Text(E->Armored ? (E->ShieldOpen>0 || E->Windup>0 ? TEXT("OPEN") : TEXT("SHIELDED")) : E->Ranged ? TEXT("CHANNELER") : TEXT("OATHBOUND"),X,Y-15,9,E->Armored ? Gold : Muted,false,true);
            if(E->WetTimer>0) Text(TEXT("WET"),X,Y+8,10,Teal,false,true);
            if(E->Windup>0) Text(TEXT("!"),X,Y-29,23,Gold,true,true);
        }
        else if(E->Boss)
        {
            Rect(452,190,536,65,Ink);
            Text(E->Enraged ? TEXT("TALOS  /  THE BROKEN OATH") : TEXT("TALOS, THE HOLLOW GUARDIAN"),720,202,16,E->Enraged ? Red : Gold,true,true);
            Bar(470,232,500,7,E->Health/E->MaxHealth,Red);
        }
    }
    for(const auto& F:G->FloatingText)
    {
        FVector P2=Project(F.Position); if(P2.Z>0) Text(F.Text,(P2.X-OX)/S,(P2.Y-OY)/S,20,F.Color.CopyWithNewOpacity(FMath::Min(1.f,F.Life)),true,true);
    }

    Rect(32,711,327,157,Ink);
    Text(TEXT("THE GODS ARE WATCHING"),52,730,11,Gold);
    Text(TEXT("DIVINE WRATH"),52,755,15,White,true);
    Text(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(G->Wrath)),324,757,13,Gold,false,true);
    Bar(52,786,286,6,G->Wrath/100,Gold);
    Wrap(G->HazardTimer>0 ? TEXT("RETRIBUTION! Leave the red circle.") : FString::Printf(TEXT("%d divine debts. More stolen powers mean faster wrath."),G->DebtCount()),52,806,287,12,G->HazardTimer>0 ? Red : Muted);

    Rect(467,750,506,119,Ink);
    const FLinearColor C=BGRules::GodColor(G->God);
    Icon(static_cast<int>(G->God),518,800,30,C);
    Text(BGRules::PowerName(G->God),563,768,18,White,true);
    auto* PC=Cast<ABGPlayerController>(GetOwningPlayerController());
    const FString PowerKey=PC->UsingGamepad ? TEXT("RT") : PC->KeyLabel(TEXT("Power"));
    Text(P->PowerCooldown>0 ? FString::Printf(TEXT("RECOVERING  /  %.1fs"),P->PowerCooldown) : TEXT("READY / ")+PowerKey,563,796,13,P->PowerCooldown>0 ? Muted : C);
    Bar(563,824,278,4,P->PowerCooldown>0 ? 1-P->PowerCooldown/P->PowerCooldownMax : 1,C);
    Border(874,780,58,44,C); Text(PowerKey,903,788,PowerKey.Len()>3 ? 11 : 23,C,true,true);
    Text(FString::Printf(TEXT("%d CURSES ACTIVE  /  R TO SWITCH POWER"),G->DebtCount()),563,841,10,Gold);
    for(int I=0;I<3;++I)
    {
        const bool Owned=G->HasGod(static_cast<EBGGod>(I));
        Button(FName(*FString::Printf(TEXT("choice%d"),I)),Owned ? FString::Printf(TEXT("[%d] %s"),I+1,*BGRules::GodName(static_cast<EBGGod>(I))) : TEXT("UNBORROWED"),467+I*170,712,166,29,Owned && static_cast<int>(G->God)==I,Owned);
    }

    Rect(1080,746,328,122,Ink);
    Text(PC->UsingGamepad ? TEXT("LEFT STICK") : PC->KeyLabel(TEXT("Forward"))+TEXT(" / ")+PC->KeyLabel(TEXT("Left")),1100,761,11,White); Text(TEXT("Move"),1260,761,13,Muted);
    Text(PC->UsingGamepad ? TEXT("X / RIGHT STICK") : PC->KeyLabel(TEXT("Attack")),1100,786,11,White); Text(TEXT("Attack"),1260,786,13,Muted);
    Text(PC->UsingGamepad ? TEXT("A") : PC->KeyLabel(TEXT("Dodge")),1100,811,11,White); Text(TEXT("Dodge"),1260,811,13,Muted);
    Text(PC->UsingGamepad ? TEXT("Y") : PC->KeyLabel(TEXT("Interact")),1100,836,11,White); Text(TEXT("Board ship"),1260,836,13,Muted);
    if(G->DamageFeedback>0) Border(8,84,1424,804,Red.CopyWithNewOpacity(G->DamageFeedback*2.8f));

    if(G->AwaitingExtraction)
    {
        FVector P2=Project(G->ExitMarker->GetActorLocation()+FVector(0,0,100));
        Text(TEXT("YOUR SHIP  [E]"),(P2.X-OX)/S,(P2.Y-OY)/S,18,Teal,true,true);
    }
    if(G->ToastLife>0)
    {
        Rect(335,645,770,56,Ink); Rect(335,645,3,56,Gold);
        Wrap(G->Toast,357,661,726,14,White,1.3);
    }
}

void ABGHUD::Offering()
{
    Rect(0,76,1440,824,Ink);
    Text(TEXT("A MOMENT BETWEEN TRIALS"),720,148,13,Gold,false,true);
    Text(TEXT("What will you owe?"),720,183,42,White,true,true);
    Text(TEXT("Borrow another god's power, or keep a little more of yourself. Curses never leave when you switch."),720,246,17,Muted,false,true);
    const FString Titles[]={TEXT("BORROW POSEIDON"),TEXT("BORROW ZEUS"),TEXT("BORROW HADES"),TEXT("A MORTAL'S RESOLVE"),TEXT("THE BRONZE OATH"),TEXT("STOLEN MOMENTS")};
    const FString Big[]={TEXT("TIDE"),TEXT("STORM"),TEXT("SOUL"),TEXT("+20"),TEXT("+8"),TEXT("20%")};
    const FString Small[]={TEXT("CONTROL + CONDUCTION SETUP"),TEXT("BURST + CONDUCTION FINISHER"),TEXT("DAMAGE + LIFE STEAL"),TEXT("MAX HEALTH / HEAL 45"),TEXT("ATTACK & POWER DAMAGE"),TEXT("SHORTER POWER COOLDOWNS")};
    const FString Desc[]={TEXT("Push, slow and wet enemies. Then switch to Zeus to conduct lightning. PRICE: faster wrath, even when another power is active."),TEXT("Strike a wide area. Wet enemies take 26 extra damage. PRICE: every divine cast now costs 8 health, whichever god you use."),TEXT("Drain nearby enemies to heal yourself. PRICE: maximum stamina becomes 75 for the rest of the voyage, even after switching."),TEXT("Refuse another divine debt. Gain 20 maximum health, heal 45 now, and shed 25 wrath. Your crew needs a captain, not another god."),TEXT("Every melee strike and divine power deals 8 more damage. A mortal's weapon can still wound a god's servant."),TEXT("Reduce every power cooldown by 20% and refresh all of them now. Your existing curses remain.")};
    const TArray<int32> Options=G->OfferingOptions();
    for(int I=0;I<3;++I)
    {
        const int Choice=Options[I];
        float X=150+I*390;
        Rect(X,316,360,360,Panel); Border(X,316,360,360,FLinearColor(.20,.29,.3));
        Icon(Choice%3,X+180,375,31,Choice<3 ? BGRules::GodColor(static_cast<EBGGod>(Choice)) : Gold); Text(Titles[Choice],X+180,433,19,White,true,true);
        Text(Big[Choice],X+180,467,43,Gold,true,true); Text(Small[Choice],X+180,521,11,Muted,false,true);
        Wrap(Desc[Choice],X+28,560,304,15,Muted);
        const FString Label=(Choice<3 ? TEXT("TAKE POWER + CURSE") : TEXT("CHOOSE MORTAL BLESSING"))+FString::Printf(TEXT("   [%d]"),I+1);
        Button(FName(*FString::Printf(TEXT("choice%d"),I)),Label,X,698,360,52,true);
    }
    Text(TEXT("YOUR VOYAGE IS PAUSED WHILE YOU CHOOSE"),720,808,11,Muted,false,true);
}

void ABGHUD::Result()
{
    bool Won=G->Phase==EBGPhase::Victory;
    Rect(0,76,1440,824,Ink);
    Icon(Won ? 0 : 2,720,173,45,Won ? Gold : Red);
    Text(Won ? TEXT("A RELIC CLAIMED. A DEBT REMAINS.") : TEXT("EVEN THE SEA RETURNS ITS DEAD."),720,246,13,Gold,false,true);
    Text(Won ? TEXT("You made it home.") : TEXT("The deep took its due."),720,280,46,White,true,true);
    Text(Won ? TEXT("Your crew pulls you aboard. Behind you, the sanctuary slips beneath the waves.") : TEXT("The ferryman saves a quarter of your obols. Your permanent upgrades endure."),720,347,16,Muted,false,true);
    const FString Values[]={FString::Printf(TEXT("+%d"),G->LastReward),FString::FromInt(G->Kills),FString::Printf(TEXT("%02d:%02d"),static_cast<int>(G->RunTime)/60,static_cast<int>(G->RunTime)%60)};
    const FString Labels[]={TEXT("OBOLS BANKED"),TEXT("OATHBOUND DEFEATED"),TEXT("TIME AT SEA")};
    for(int I=0;I<3;++I) { float X=275+I*305; Rect(X,411,280,129,Panel); Text(Values[I],X+140,432,38,Gold,true,true); Text(Labels[I],X+140,494,12,Muted,false,true); }
    Text(FString::Printf(TEXT("HOMEWARD CHART / %d OF 3    -    TOMAS / %s"),G->Save->HomewardProgress,*BGRules::SailorState(G->Save->SailorRecovery)),720,570,15,Gold,false,true);
    Wrap(G->VoyageConsequence,330,612,780,21,White);
    Text(FString::Printf(TEXT("%d MEDICINE DELIVERED / %s"),G->Save->LastMedicine,!G->BargainResolved ? TEXT("NO BARGAIN MADE") : G->TributeAccepted ? TEXT("TRIBUTE PAID") : TEXT("TRIBUTE REFUSED")),720,690,12,Muted,false,true);
    Button(TEXT("harbor"),TEXT("RETURN TO THE ARGO   /   ENTER"),430,733,580,58,true);
    Text(G->SaveSucceeded ? TEXT("PROGRESS SAVED") : TEXT("SAVE FAILED - CHECK DISK SPACE"),720,825,11,G->SaveSucceeded ? Teal : Red,false,true);
}

void ABGHUD::Journal()
{
    Rect(0,76,1440,824,Ink);
    Text(TEXT("THE OATHBREAKER'S JOURNAL"),64,112,13,Gold);
    Text(TEXT("Every gift has a cost."),61,143,42,White,true);
    Button(TEXT("close"),TEXT("CLOSE   /   TAB"),1176,122,200,43);
    Text(TEXT("01  /  THE STORY SO FAR"),64,226,13,Gold);
    Wrap(TEXT("Your ship has been stranded in a sea the gods have sealed. Your crew calls you captain. The temples call you thief. Somewhere beyond these islands, the people you promised to return to are still waiting."),64,259,388,18,White);
    Wrap(TEXT("At the Drowned Sanctuary, Talos guards a relic that can quiet the sea. Borrow a divine power, break his watch, and bring the relic back to your ship."),64,448,388,17,Muted);
    Text(TEXT("THE RULE OF THE VOYAGE"),64,613,12,Gold);
    Wrap(TEXT("Blessings last one expedition. Banked obols and ship upgrades last forever. Escape with everything, or lose three quarters of what you carried."),64,645,388,17,Muted);
    const FString Lore[]={TEXT("The sea never forgets a debt."),TEXT("Lightning always takes the shortest path."),TEXT("What is borrowed must be returned.")};
    const FString Power[]={TEXT("Q / 42 damage in a 580-unit radius. Pushes enemies back and slows them for 3 seconds. 6-second base cooldown."),TEXT("Q / 62 damage in a 740-unit radius. The widest and strongest burst. 7-second base cooldown."),TEXT("Q / 38 damage in a 440-unit radius. Heal 12 per enemy hit, up to 30. 8-second base cooldown.")};
    const FString Curse[]={TEXT("Wrath grows faster while this debt is carried. Tidebreak wets enemies for Zeus's +26 conduction damage."),TEXT("Every divine cast costs 8 health once Zeus is borrowed. Changing masks never removes this price."),TEXT("Maximum stamina stays at 75 once Hades is borrowed. Changing masks never removes this price.")};
    for(int I=0;I<3;++I)
    {
        float Y=226+I*192;
        Rect(502,Y,874,174,Panel); Icon(I,547,Y+45,25,BGRules::GodColor(static_cast<EBGGod>(I)));
        Text(BGRules::GodName(static_cast<EBGGod>(I))+TEXT("  /  ")+BGRules::PowerName(static_cast<EBGGod>(I)),594,Y+21,19,White,true);
        Text(Lore[I],594,Y+50,13,Muted);
        Wrap(Power[I],594,Y+77,742,15,White);
        Wrap(TEXT("CURSE / ")+Curse[I],594,Y+127,742,13,Gold);
    }
    Wrap(TEXT("YOUR BURDENS / ")+G->BurdenSummary(),502,815,860,13,Gold);
}

void ABGHUD::Crew()
{
    Rect(0,76,1440,824,Ink);
    Text(TEXT("ABOARD THE ARGO"),64,116,13,Gold);
    Text(TEXT("Who are you coming home for?"),61,149,39,White,true);
    Button(TEXT("close"),TEXT("BACK   /   ESC"),1176,116,200,43);
    const FString Names[]={TEXT("IONE / NAVIGATOR"),TEXT("NERA / SHIP'S HEALER"),TEXT("DAEMON / BRONZESMITH")};
    const FString Hints[]={TEXT("The stars. Her daughter. The way home."),TEXT("Your wounds. Your debts. Your name."),TEXT("Your blade. The cost of a miracle.")};
    for(int I=0;I<3;++I)
    {
        float Y=255+I*112;
        Button(FName(*FString::Printf(TEXT("crew%d"),I)),Names[I],64,Y,302,54,G->CrewSpeaker==I);
        Text(Hints[I],76,Y+68,12,Muted);
    }
    Rect(414,255,962,334,Panel); Rect(414,255,3,334,Gold);
    Text(Names[G->CrewSpeaker],450,281,15,Gold);
    Wrap(G->CrewDialogue(),450,325,885,22,White,1.55f);
    Text(FString::Printf(TEXT("HOMEWARD CHART / %d OF 3 FRAGMENTS"),G->Save->HomewardProgress),64,644,13,Gold);
    for(int I=0;I<3;++I) { Diamond(92+I*100,705,19,I<G->Save->HomewardProgress ? Gold : Muted); if(I<2) Line(118+I*100,705,164+I*100,705,Muted); }
    Text(TEXT("TOMAS / ")+BGRules::SailorState(G->Save->SailorRecovery),64,774,14,Teal);
    Text(FString::Printf(TEXT("%d MEDICINE DELIVERED"),G->Save->MedicineDelivered),64,800,11,Muted);
    Text(TEXT("THE SHIP REMEMBERS"),414,628,13,Gold);
    if(G->Save->VoyageJournal.IsEmpty()) Wrap(TEXT("No voyage has returned yet. Your crew is waiting for the first story worth writing down."),414,663,885,17,Muted);
    else
    {
        const int Start=FMath::Max(0,G->Save->VoyageJournal.Num()-3);
        for(int I=Start;I<G->Save->VoyageJournal.Num();++I) { float Y=664+(I-Start)*51; Line(414,Y+31,1376,Y+31,Muted.CopyWithNewOpacity(.25)); Text(G->Save->VoyageJournal[I],430,Y,15,White); }
    }
    Text(TEXT("Every recovered relic reveals another piece of home. Every voyage leaves a story."),414,834,14,Muted);
}

void ABGHUD::Pause()
{
    Rect(0,76,1440,824,FLinearColor(.01,.025,.035,.93));
    Rect(460,154,520,650,Panel); Border(460,154,520,650,FLinearColor(.22,.29,.30));
    Diamond(720,220,28,Gold); Text(TEXT("A quiet moment."),720,278,35,White,true,true);
    Text(TEXT("THE WORLD CAN WAIT."),720,330,12,Muted,false,true);
    Button(TEXT("close"),TEXT("RESUME   /   ESC"),508,385,424,54,true);
    Button(TEXT("help"),TEXT("CONTROLS & HOW TO PLAY"),508,455,424,48);
    Button(TEXT("settings"),TEXT("INPUT & ACCESSIBILITY"),508,519,424,48);
    if(G->HasActiveVoyage()) Button(TEXT("abandon"),TEXT("END VOYAGE  /  KEEP 25% OF OBOLS"),508,583,424,48);
    Button(TEXT("quit"),TEXT("SAVE & QUIT"),508,665,424,48);
    Text(TEXT("Banked progress is saved. Active voyages do not resume."),720,749,12,Muted,false,true);
}

void ABGHUD::Help()
{
    Rect(0,76,1440,824,Ink);
    Text(TEXT("YOUR FIRST VOYAGE"),80,120,13,Gold); Text(TEXT("Borrow wisely. Move constantly."),77,155,39,White,true);
    Button(TEXT("close"),TEXT("CLOSE   /   ESC"),1170,127,190,43);
    auto* PC=Cast<ABGPlayerController>(GetOwningPlayerController());
    const FString Keys[]={PC->KeyLabel(TEXT("Forward"))+TEXT(" ")+PC->KeyLabel(TEXT("Left"))+TEXT(" ")+PC->KeyLabel(TEXT("Back"))+TEXT(" ")+PC->KeyLabel(TEXT("Right")),PC->KeyLabel(TEXT("Attack")),PC->KeyLabel(TEXT("Dodge")),PC->KeyLabel(TEXT("Power")),PC->KeyLabel(TEXT("Cycle"))+TEXT(" / 1 2 3"),PC->KeyLabel(TEXT("Interact")),PC->KeyLabel(TEXT("Journal"))+TEXT(" / ")+PC->KeyLabel(TEXT("Crew")),PC->KeyLabel(TEXT("Pause"))};
    const FString Actions[]={TEXT("Move relative to the camera"),TEXT("Aim / hold for three-hit combo"),TEXT("Dodge / costs 30 stamina"),TEXT("Use the active stolen power"),TEXT("Switch between borrowed powers"),TEXT("Board at the extraction beacon"),TEXT("Journal / crew; pauses combat"),TEXT("Pause / resume")};
    for(int I=0;I<8;++I) { float Y=255+I*60; Rect(80,Y,655,49,Panel); Text(Keys[I],98,Y+16,14,Gold); Text(Actions[I],322,Y+16,15,White); }
    Text(TEXT("READ THE FIGHT"),792,258,15,Gold);
    Wrap(TEXT("Circles mark incoming strikes. Move before they land. Sentinels guard against ordinary melee: finish the combo or strike during their windup. Purple channelers retreat when approached."),792,293,535,18,White);
    Text(TEXT("TAKE WHAT YOU NEED"),792,418,15,Gold);
    Wrap(TEXT("Borrow powers or refuse their debts. Curses stack. Wet enemies with Tidebreak, then switch to Stormcall. Carry medicine through Talos's fight to help Tomas; tribute trades one flask for stronger powers."),792,454,535,18,White);
    Text(TEXT("KEEP WHAT YOU EARN"),792,580,15,Gold);
    Wrap(TEXT("An escape banks all carried obols. Death or ending a voyage banks one quarter. Spend banked obols on health and weapon upgrades at the ship."),792,616,535,19,White);
    Text(TEXT("CONTROLLER / STICKS MOVE & AIM / X ATTACK / A DODGE / RT POWER / LB-RB SWITCH / Y BOARD"),80,785,13,Gold);
    Text(TEXT("D-PAD & A navigate menus. Pause > Input & Accessibility to rebind keys or enable assistance."),80,822,14,Muted);
}

void ABGHUD::Decision()
{
    Rect(0,76,1440,824,Ink);
    const bool Brief=G->Phase==EBGPhase::Briefing, Route=G->Phase==EBGPhase::Route, Relic=G->Phase==EBGPhase::Relic;
    Text(Brief ? TEXT("BEFORE YOU LEAVE THE ARGO") : Route ? TEXT("ONE ISLAND. TWO PROMISES.") : Relic ? TEXT("DAEMON'S LAST PIECE OF ADVICE") : TEXT("POSEIDON HAS NOTICED YOUR THEFT"),720,131,13,Gold,false,true);
    Text(Brief ? TEXT("Bring him something mortal.") : Route ? TEXT("Which risk will you carry?") : Relic ? TEXT("Make the gift your own.") : TEXT("A flask for a miracle."),720,169,42,White,true,true);
    if(Brief)
    {
        Rect(160,270,1120,383,Panel); Icon(2,241,340,38,Teal);
        Text(TEXT("NERA / THE SHIP'S HEALER"),320,299,16,Gold);
        Wrap(G->Save->SailorRecovery>=2 ? TEXT("Tomas is back on deck, captain. But he will not be our last wounded sailor. The sanctuary still holds medicine. Bring some aboard before the sea gives us another reason to need it.") : G->Save->SailorRecovery==1 ? TEXT("Tomas held his cup this morning. That first dose mattered, captain. He is recovering, but he needs one more to stand beside us again. The infirmary holds enough. The vault could leave him a dose even after tribute. Bring him the rest of the way home.") : TEXT("Tomas held the mast together when the sea struck us. Now he cannot hold a cup. His fever is rising. There is medicine in the sanctuary's infirmary, and more in its guarded vault. Bring him a dose, captain. Two will put him on his feet."),320,341,894,23,White,1.5f);
        Line(200,524,1240,524,Muted.CopyWithNewOpacity(.3));
        Wrap(TEXT("The safe path holds one flask. The vault holds two behind shielded sentinels. Poseidon will ask for one as tribute. Medicine reaches the ship only if you escape."),200,553,1030,18,Muted);
        Text(TEXT("TOMAS / ")+BGRules::SailorState(G->Save->SailorRecovery),200,624,13,Teal);
        Button(TEXT("cancelvoyage"),TEXT("STAY ABOARD / ESC"),260,724,370,57);
        Button(TEXT("depart"),TEXT("I WILL BRING IT HOME / ENTER"),654,724,526,57,true);
    }
    else
    {
        Text(Route ? TEXT("Both routes lead to Talos. Only you decide what the crew might receive.") : Relic ? TEXT("One relic for this voyage. A different habit, not just a stronger number.") : TEXT("Your medicine is real. So is the power he offers. Both choices can bring you home."),720,236,17,Muted,false,true);
        TArray<FString> Titles,Big,Desc,Labels;
        if(Route)
        {
            Titles={TEXT("THE HEALER'S PATH"),TEXT("THE GUARDED VAULT")}; Big={TEXT("ONE FLASK"),TEXT("TWO FLASKS")};
            Desc={TEXT("Three oathbound guard the infirmary. A shorter fight, one dose for Tomas. Paying tribute later will leave no medicine for the ship."),TEXT("Five enemies, including three shielded sentinels, guard the vault. Two doses can cure Tomas, or leave one for him after paying Poseidon.")};
            Labels={TEXT("TAKE THE SAFER PATH [1]"),TEXT("ATTEMPT THE EXTRA THEFT [2]")};
        }
        else if(Relic)
        {
            Titles={TEXT("STORMGLASS"),TEXT("FERRYMAN'S THREAD"),TEXT("OATHSTONE")}; Big={TEXT("ARC"),TEXT("EVADE"),TEXT("BREAK")};
            Desc={TEXT("When Stormcall consumes Wet, lightning arcs for 18 damage to other enemies within 320 units. Gather a crowd, wet it, then switch to Zeus. Needs both gods."),TEXT("Dodge from inside an enemy's warning during its final 0.35 seconds to heal 8 and shed 12 wrath. A 1.5-second recovery prevents repeated rewards."),TEXT("Your third melee strike interrupts ordinary enemies and restores 24 stamina when it connects. It opens sentinel shields. Talos cannot be interrupted.")};
            Labels={TEXT("TAKE STORMGLASS [1]"),TEXT("TAKE THE THREAD [2]"),TEXT("TAKE OATHSTONE [3]")};
        }
        else
        {
            Titles={TEXT("PAY HIS TRIBUTE"),TEXT("KEEP YOUR PROMISE")}; Big={TEXT("POWER"),TEXT("MEDICINE")};
            Desc={TEXT("Give Poseidon one flask. All divine powers gain 20 damage, and Tidebreak gains 100 range for this voyage. No sea strikes. You will carry ")+FString::FromInt(FMath::Max(0,G->Medicine-1))+TEXT(" flasks into the sanctum."),TEXT("Keep every flask for Nera. Poseidon marks three sea strikes during the guardian fight. Each gives you time to leave the ground. You will carry ")+FString::FromInt(G->Medicine)+TEXT(" flasks into the sanctum.")};
            Labels={TEXT("PAY ONE FLASK [1]"),TEXT("REFUSE THE TRIBUTE [2]")};
        }
        const int Count=Titles.Num(); const float W=Count==3 ? 360 : 520; const float Start=Count==3 ? 150 : 180;
        for(int I=0;I<Count;++I)
        {
            const float X=Start+I*(W+30);
            Rect(X,311,W,361,Panel); Border(X,311,W,361,Muted.CopyWithNewOpacity(.3));
            Icon(I,X+W/2,362,29,I==0 ? Gold : Teal);
            Text(Titles[I],X+W/2,418,19,White,true,true);
            Text(Big[I],X+W/2,452,40,Gold,true,true);
            Wrap(Desc[I],X+28,526,W-56,17,Muted);
            Button(FName(*FString::Printf(TEXT("choice%d"),I)),Labels[I],X,698,W,54,true);
        }
    }
    Text(TEXT("THE WORLD WAITS WHILE YOU CHOOSE / D-PAD TO FOCUS, A TO CONFIRM"),720,828,12,Muted,false,true);
}

void ABGHUD::Settings()
{
    Rect(0,76,1440,824,Ink);
    auto* PC=Cast<ABGPlayerController>(GetOwningPlayerController());
    Text(TEXT("INPUT & ACCESSIBILITY"),64,113,13,Gold);
    Text(TEXT("Make room for yourself."),61,146,38,White,true);
    Button(TEXT("close"),TEXT("BACK / ESC"),1176,115,200,43);
    const TArray<FName> Actions=ABGPlayerController::BindingActions();
    for(int I=0;I<Actions.Num();++I)
    {
        const float X=64+(I/7)*680, Y=235+(I%7)*54;
        Text(Actions[I].ToString().ToUpper(),X+10,Y+13,14,White);
        Button(FName(*(TEXT("bind_")+Actions[I].ToString())),Actions[I]==TEXT("Pause") ? TEXT("ESCAPE / RESERVED") : PC->Rebinding==Actions[I] ? TEXT("PRESS A KEY / ESC CANCELS") : PC->KeyLabel(Actions[I]),X+170,Y,450,43,PC->Rebinding==Actions[I],Actions[I]!=TEXT("Pause"));
    }
    Text(TEXT("Click an action, then press a key. Existing bindings swap. 1 / 2 / 3 and ESC remain reserved."),64,627,13,Muted);
    Button(TEXT("motion"),G->Save->ReducedMotion ? TEXT("REDUCED MOTION: ON") : TEXT("REDUCED MOTION: OFF"),64,674,404,48,G->Save->ReducedMotion);
    Button(TEXT("assist"),G->Save->Assistance ? TEXT("ASSISTANCE: ON") : TEXT("ASSISTANCE: OFF"),488,674,404,48,G->Save->Assistance);
    Button(TEXT("sound"),G->Save->Muted ? TEXT("SOUND: OFF") : TEXT("SOUND: ON"),912,674,404,48);
    Wrap(TEXT("Reduced motion removes hit-stop and camera smoothing. Assistance reduces incoming damage by 35% and extends enemy warnings by 25%. Rewards stay the same."),64,741,884,16,Muted);
    Button(TEXT("resetkeys"),TEXT("RESET KEY BINDINGS"),978,749,338,44);
    Text(TEXT("CONTROLLER / LEFT STICK MOVE, RIGHT STICK AIM, X ATTACK, A DODGE, RT POWER, LB/RB SWITCH, Y BOARD"),64,834,12,Gold);
}

void ABGHUD::DrawHUD()
{
    Super::DrawHUD();
    G=GetWorld()->GetAuthGameMode<ABGGameMode>();
    if(!G || !G->Save || !Canvas || !G->Player()) return;
    if(G->Phase!=EBGPhase::Combat || G->Paused || G->CrewOpen || G->JournalOpen || G->HelpOpen || G->SettingsOpen)
        DrawRect(Ink,0,0,Canvas->ClipX,Canvas->ClipY);
    MenuActions.Reset();
    const FName Context=G->SettingsOpen ? FName(TEXT("settings")) : G->Paused ? FName(TEXT("pause")) : G->CrewOpen ? FName(TEXT("crew")) : G->HelpOpen ? FName(TEXT("help")) : G->JournalOpen ? FName(TEXT("journal")) : FName(*FString::FromInt(static_cast<int>(G->Phase)));
    if(Context!=FocusContext)
    {
        FocusContext=Context;
        FocusAction=G->Paused || G->HelpOpen || G->JournalOpen || G->CrewOpen || G->SettingsOpen ? TEXT("close") : G->Phase==EBGPhase::Harbor ? TEXT("sail") : G->Phase==EBGPhase::Briefing ? TEXT("depart") : G->Phase==EBGPhase::Victory || G->Phase==EBGPhase::Defeat ? TEXT("harbor") : TEXT("choice0");
    }
    S=FMath::Min(Canvas->ClipX/1440.f,Canvas->ClipY/900.f);
    OX=(Canvas->ClipX-1440*S)/2; OY=(Canvas->ClipY-900*S)/2;
    float MX=0,MY=0; GetOwningPlayerController()->GetMousePosition(MX,MY); Mouse=FVector2D((MX-OX)/S,(MY-OY)/S);
    if(G->Phase==EBGPhase::Harbor) Harbor();
    else if(G->Phase==EBGPhase::Combat) Combat();
    else { Header(TEXT("CHAPTER I / DROWNED SANCTUARY")); if(G->Phase==EBGPhase::Offering) Offering(); else if(G->Phase==EBGPhase::Victory || G->Phase==EBGPhase::Defeat) Result(); else Decision(); }
    if(G->JournalOpen || G->Paused || G->HelpOpen || G->CrewOpen || G->SettingsOpen) { HitBoxMap.Reset(); HitBoxHits.Reset(); MenuActions.Reset(); }
    if(G->JournalOpen) Journal();
    if(G->Paused) Pause();
    if(G->HelpOpen) Help();
    if(G->CrewOpen) Crew();
    if(G->SettingsOpen) Settings();
    if(!MenuActions.Contains(FocusAction) && !MenuActions.IsEmpty()) FocusAction=MenuActions[0];
    if(G->Phase==EBGPhase::Harbor && G->ToastLife>0 && !G->Paused && !G->JournalOpen && !G->HelpOpen && !G->CrewOpen && !G->SettingsOpen)
    {
        Rect(285,830,870,45,Ink); Border(285,830,870,45,Gold.CopyWithNewOpacity(.5)); Text(G->Toast,720,845,13,White,false,true);
    }
}
void ABGHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    if(auto* Mode=GetWorld()->GetAuthGameMode<ABGGameMode>()) Mode->HandleAction(BoxName);
}
