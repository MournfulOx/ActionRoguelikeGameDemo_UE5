#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

class UStaticMeshComponent;
class URadialForceComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	AExplosiveBarrel();
    
	bool bExploded = false;

	UFUNCTION(BlueprintCallable)
	void Explode();
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
	
	UFUNCTION()
	void OnBarrelHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	   AController* EventInstigator, AActor* DamageCauser) override;

protected:

	UPROPERTY(VisibleAnywhere)
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ExplodeEffect;

	FTimerHandle TimerHandle_Destroy;
	void DestroyBarrel();

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};