#include "ExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AExplosiveBarrel::AExplosiveBarrel()
{
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
    RootComponent = MeshComp;
    MeshComp->SetCollisionProfileName("PhysicsActor");
    MeshComp->SetSimulatePhysics(true);

    RadialForceComp = CreateDefaultSubobject<URadialForceComponent>("RadialForceComp");
    RadialForceComp->SetupAttachment(RootComponent);
    RadialForceComp->Radius = 600.0f;
    RadialForceComp->bImpulseVelChange = true;
    RadialForceComp->bAutoActivate = false;
    RadialForceComp->ImpulseStrength = 3000.0f;
}

void AExplosiveBarrel::BeginPlay()
{
    Super::BeginPlay();
    MeshComp->OnComponentHit.AddDynamic(this, &AExplosiveBarrel::OnBarrelHit);
}

void AExplosiveBarrel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

float AExplosiveBarrel::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Explode();
    return ActualDamage;
}


void AExplosiveBarrel::Explode()
{
    if (bExploded) return;
    bExploded = true;

    RadialForceComp->FireImpulse();

    for (TActorIterator<AExplosiveBarrel> It(GetWorld()); It; ++It)
    {
        AExplosiveBarrel* OtherBarrel = *It;
        if (OtherBarrel && OtherBarrel != this && !OtherBarrel->bExploded)
        {
            float Distance = FVector::Dist(GetActorLocation(), OtherBarrel->GetActorLocation());
            if (Distance <= RadialForceComp->Radius)
            {
                float Delay = Distance / 500.0f;
                FTimerHandle TimerHandle_Chain;
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindUObject(OtherBarrel, &AExplosiveBarrel::Explode);
                GetWorldTimerManager().SetTimer(TimerHandle_Chain, TimerDelegate, Delay, false);
            }
        }
    }

    UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeEffect, GetActorLocation());
    MeshComp->SetVisibility(false);
    // 不立刻关碰撞，让冲量先传播
    GetWorldTimerManager().SetTimer(TimerHandle_Destroy, this, &AExplosiveBarrel::DestroyBarrel, 0.2f);
}

void AExplosiveBarrel::DestroyBarrel()
{
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Destroy();
}

void AExplosiveBarrel::OnBarrelHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 撞击力超过阈值才爆炸，普通放置不会触发
    if (NormalImpulse.Size() > 100000.0f)
    {
        Explode();
    }
}