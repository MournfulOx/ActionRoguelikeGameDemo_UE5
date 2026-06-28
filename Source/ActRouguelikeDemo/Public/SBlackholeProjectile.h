#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBlackholeProjectile.generated.h"

class USphereComponent;
class UParticleSystemComponent;
class UProjectileMovementComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API ASBlackholeProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASBlackholeProjectile();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* EffectComp;

	// 吸引范围（大圈）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PullSphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* MovementComp;

	// 逻辑上的吸引半径（不等于球体组件半径）
	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float PullRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float PullStrength = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float DamagePerSecond = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float LifeSpanDuration = 5.0f;

	// 飞行距离上限，到达后停止移动
	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float MaxRange = 1500.0f;

	FVector SpawnLocation;
	FTimerHandle TimerHandle_DamageTick;

	void ApplyPullDamage();

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal,
		FVector NormalImpulse, const FHitResult& Hit) override;
};
