#pragma once

#include "CoreMinimal.h"
#include "SPowerupActor.h"
#include "SCoin.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASCoin : public ASPowerupActor
{
	GENERATED_BODY()

public:
	ASCoin();

protected:
	UPROPERTY(EditAnywhere, Category = "Powerup")
	int32 CreditsAmount;

	void OnActivated_Implementation(APawn* InstigatorPawn) override;
};
