#pragma once

#include "CoreMinimal.h"
#include "SProjectileBase.h"
#include "SAIProjectile.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASAIProjectile : public ASProjectileBase
{
	GENERATED_BODY()

public:
	ASAIProjectile();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float DamageAmount = 5.0f;

	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal,
		FVector NormalImpulse, const FHitResult& Hit) override;
};
