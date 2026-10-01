#pragma once
#include "CoreMinimal.h"
#include "BGTypes.generated.h"

UENUM(BlueprintType)
enum class EBGPhase : uint8 { Harbor, Briefing, Combat, Route, Relic, Bargain, Offering, Victory, Defeat };

UENUM(BlueprintType)
enum class EBGRelic : uint8 { None, Stormglass, FerrymanThread, Oathstone };

UENUM(BlueprintType)
enum class EBGGod : uint8 { Poseidon, Zeus, Hades };

namespace BGRules
{
    inline FString RelicName(EBGRelic Relic) { return Relic==EBGRelic::Stormglass ? TEXT("STORMGLASS") : Relic==EBGRelic::FerrymanThread ? TEXT("FERRYMAN'S THREAD") : Relic==EBGRelic::Oathstone ? TEXT("OATHSTONE") : TEXT("NO RELIC"); }
    inline FString SailorState(int32 State) { return State>=2 ? TEXT("FIT FOR DUTY") : State==1 ? TEXT("RECOVERING") : TEXT("FEVERISH"); }
    inline int32 UpgradeCost(int32 Level) { return 30 + FMath::Clamp(Level, 0, 5) * 25; }
    inline int32 BankedReward(int32 Carried, bool Escaped) { return Escaped ? FMath::Max(0, Carried) : FMath::Max(0, Carried) / 4; }
    inline float MaxHealth(int32 HullLevel) { return 100.f + FMath::Clamp(HullLevel, 0, 5) * 15.f; }
    inline float MaxStamina(EBGGod God) { return God == EBGGod::Hades ? 75.f : 100.f; }
    inline FString GodName(EBGGod God) { return God == EBGGod::Poseidon ? TEXT("POSEIDON") : God == EBGGod::Zeus ? TEXT("ZEUS") : TEXT("HADES"); }
    inline FString PowerName(EBGGod God) { return God == EBGGod::Poseidon ? TEXT("TIDEBREAK") : God == EBGGod::Zeus ? TEXT("STORMCALL") : TEXT("SOUL HARVEST"); }
    inline FLinearColor GodColor(EBGGod God) { return God == EBGGod::Poseidon ? FLinearColor(0.25f,0.85f,0.83f) : God == EBGGod::Zeus ? FLinearColor(0.85f,0.73f,1.f) : FLinearColor(1.f,0.55f,0.40f); }
    inline bool InMeleeArc(const FVector& Facing, const FVector& Delta, float Radius)
    {
        return Delta.SizeSquared2D() <= FMath::Square(Radius) && (Delta.SizeSquared2D() < 2500.f || FVector::DotProduct(Facing.GetSafeNormal2D(), Delta.GetSafeNormal2D()) > 0.15f);
    }
}
