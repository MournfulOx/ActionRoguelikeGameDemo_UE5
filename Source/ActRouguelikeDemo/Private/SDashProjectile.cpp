// SDashProjectile.cpp
#include "SDashProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASDashProjectile::ASDashProjectile()
{
}

void ASDashProjectile::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(TimerHandle_Detonate, this, &ASDashProjectile::Explode, 2.0f);
}

void ASDashProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
    bool bSelfMoved, FVector HitLocation, FVector HitNormal,
    FVector NormalImpulse, const FHitResult& Hit)
{
    Explode();
}

void ASDashProjectile::Explode()
{
    if (bExploded) return;
    bExploded = true;

    GetWorldTimerManager().ClearTimer(TimerHandle_Detonate);
    MovementComp->StopMovementImmediately();
    UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, GetActorLocation());
    EffectComp->DeactivateSystem();
    GetWorldTimerManager().SetTimer(TimerHandle_Teleport, this, &ASDashProjectile::TeleportInstigator, TeleportDelay);
}

void ASDashProjectile::TeleportInstigator()
{
    AActor* InstigatorActor = GetInstigator();
    UE_LOG(LogTemp, Warning, TEXT("TeleportInstigator called, Instigator: %s"), *GetNameSafe(InstigatorActor));
    if (InstigatorActor)
    {
        UGameplayStatics::SpawnEmitterAtLocation(this, TeleportExitEffect, InstigatorActor->GetActorLocation());
        InstigatorActor->TeleportTo(GetActorLocation() + FVector(0, 0, 50.0f), InstigatorActor->GetActorRotation());
    }
    Destroy();
}