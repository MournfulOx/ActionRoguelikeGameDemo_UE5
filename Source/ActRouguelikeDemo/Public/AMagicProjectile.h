#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AMagicProjectile.generated.h"


class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API AAMagicProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAMagicProjectile();

protected:
	
	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// 设为 false 可禁用 Block 碰撞时的伤害+销毁（黑洞蓝图关掉此项）
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	bool bDestroyOnBlockingHit = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* EffectComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ImpactEffect;
	
};
