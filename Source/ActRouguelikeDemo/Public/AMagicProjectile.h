#pragma once

#include "CoreMinimal.h"
#include "SProjectileBase.h"
#include "AMagicProjectile.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API AAMagicProjectile : public ASProjectileBase
{
	GENERATED_BODY()

public:
	AAMagicProjectile();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	bool bDestroyOnBlockingHit = true;

	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal,
		FVector NormalImpulse, const FHitResult& Hit) override;
};
