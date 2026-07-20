#include "SProjectileBase.h"
#include "SActionComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASProjectileBase::ASProjectileBase()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");
	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->SetSphereRadius(20.0f);
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	AudioComp = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	AudioComp->SetupAttachment(RootComponent);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;
}

bool ASProjectileBase::TryParryReflect(AActor* OtherActor)
{
	USActionComponent* ActionComp = Cast<USActionComponent>(OtherActor->GetComponentByClass(USActionComponent::StaticClass()));
	if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
	{
		MovementComp->Velocity = -MovementComp->Velocity;
		SetInstigator(Cast<APawn>(OtherActor));
		return true;
	}
	return false;
}

void ASProjectileBase::Explode_Implementation()
{
	if (ensure(!IsActorBeingDestroyed()))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), ImpactCameraShake, GetActorLocation(), 0, 1000, 1);
		Destroy();
	}
}
