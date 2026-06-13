#pragma once

#include "CoreMinimal.h"
#include "SPowerupActor.h"
#include "SHealthPotion.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASHealthPotion : public ASPowerupActor
{
	GENERATED_BODY()

public:
	ASHealthPotion();

protected:
	UPROPERTY(EditAnywhere, Category = "Powerup")
	float HealAmount;

	void OnActivated_Implementation(APawn* InstigatorPawn) override;
};
