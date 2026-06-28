#include "SDashProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASDashProjectile::ASDashProjectile()
{
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASDashProjectile::OnDashOverlap);
}

void ASDashProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 生成瞬间关闭碰撞，0.1s 后再开启，避免紧贴玩家时立即触发
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().SetTimer(TimerHandle_EnableCollision, this, &ASDashProjectile::EnableCollision, 0.1f);

	GetWorldTimerManager().SetTimer(TimerHandle_Detonate, this, &ASDashProjectile::Explode_Implementation, 1.0f);
}

void ASDashProjectile::EnableCollision()
{
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ASDashProjectile::OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		Explode();
	}
}

void ASDashProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (Other && Other != GetInstigator())
	{
		Explode();
	}
}

void ASDashProjectile::Explode_Implementation()
{
	if (bExploded) return;
	bExploded = true;

	GetWorldTimerManager().ClearTimer(TimerHandle_Detonate);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComp->StopMovementImmediately();
	UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation());
	EffectComp->DeactivateSystem();
	GetWorldTimerManager().SetTimer(TimerHandle_Teleport, this, &ASDashProjectile::TeleportInstigator, TeleportDelay);
}

void ASDashProjectile::TeleportInstigator()
{
	AActor* InstigatorActor = GetInstigator();
	if (!ensure(InstigatorActor))
	{
		Destroy();
		return;
	}

	FVector TeleportTarget = GetActorLocation() + FVector(0, 0, 100.0f);

	UGameplayStatics::SpawnEmitterAtLocation(this, TeleportExitEffect, InstigatorActor->GetActorLocation());
	InstigatorActor->TeleportTo(TeleportTarget, InstigatorActor->GetActorRotation());
	Destroy();
}
