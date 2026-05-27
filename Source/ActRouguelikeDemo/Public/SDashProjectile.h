// SDashProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "AMagicProjectile.h"
#include "SDashProjectile.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASDashProjectile : public AAMagicProjectile
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

	virtual void BeginPlay() override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal,
		FVector NormalImpulse, const FHitResult& Hit) override;

	void Explode();
	void TeleportInstigator();
};