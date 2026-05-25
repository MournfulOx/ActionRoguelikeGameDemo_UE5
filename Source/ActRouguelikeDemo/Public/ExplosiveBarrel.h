// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

// 前向声明：告诉编译器这两个类存在，不需要 #include
class UStaticMeshComponent;
class URadialForceComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	AExplosiveBarrel();

public:

	UFUNCTION(BlueprintCallable)
	void Explode();

protected:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	URadialForceComponent* RadialForceComp;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
							 AController* EventInstigator, AActor* DamageCauser) override;

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

};
