#pragma once

#include "CoreMinimal.h"
#include "SProjectileBase.h"
#include "SDashProjectile.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASDashProjectile : public ASProjectileBase
{
	GENERATED_BODY()

public:
	ASDashProjectile();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float TeleportDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	UParticleSystem* TeleportExitEffect;

	bool bExploded = false;

	FTimerHandle TimerHandle_Detonate;
	FTimerHandle TimerHandle_Teleport;
	FTimerHandle TimerHandle_EnableCollision;

	void EnableCollision();

	virtual void BeginPlay() override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal,
		FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION()
	void OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	virtual void Explode_Implementation() override;

	void TeleportInstigator();
};
