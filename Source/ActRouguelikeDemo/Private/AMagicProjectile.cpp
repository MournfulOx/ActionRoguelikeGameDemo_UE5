#include "AMagicProjectile.h"

#include "SAttributeComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AAMagicProjectile::AAMagicProjectile()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");
	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAMagicProjectile::OnActorOverlap);
	SphereComp->SetSphereRadius(20.0f);
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;
	
}


void AAMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									   bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		UE_LOG(LogTemp, Log, TEXT("Overlap: %s hit %s"), *GetName(), *OtherActor->GetName());

		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (AttributeComp)
		{
			AttributeComp->ApplyHealthChange(GetInstigator(), -20.0f);
		}

		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation());
		Destroy();
	}
}

void AAMagicProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bDestroyOnBlockingHit) return;

	// Block 碰撞（如炸桶 PhysicsBody），Overlap 已处理角色，不重复处理
	if (Other && Other != GetInstigator())
	{
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			Other->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (!AttributeComp)
		{
			AController* InstigatorController = GetInstigator() ? GetInstigator()->GetInstigatorController() : nullptr;
			UGameplayStatics::ApplyDamage(Other, 20.0f, InstigatorController, this, nullptr);
			UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation());
			Destroy();
		}
	}
}
