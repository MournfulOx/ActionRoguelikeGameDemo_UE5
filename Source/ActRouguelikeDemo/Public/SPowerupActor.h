#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGameplayInterface.h"
#include "SPowerupActor.generated.h"

class USphereComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API ASPowerupActor : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()

public:
	ASPowerupActor();

	void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Powerup")
	float CooldownDuration;

	FTimerHandle TimerHandle_Cooldown;

	void HideAndCooldown();

	UFUNCTION()
	void RestorePowerup();

	UFUNCTION(BlueprintNativeEvent, Category = "Powerup")
	void OnActivated(APawn* InstigatorPawn);
	virtual void OnActivated_Implementation(APawn* InstigatorPawn);
};
