#include "SBlackholeProjectile.h"
#include "SAttributeComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AI/SAICharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "EngineUtils.h"

ASBlackholeProjectile::ASBlackholeProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	PullSphereComp = CreateDefaultSubobject<USphereComponent>("PullSphereComp");
	PullSphereComp->SetSphereRadius(20.0f);	// 小球体只用于停止飞行，吸引范围由 PullRadius 控制
	PullSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PullSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	PullSphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootComponent = PullSphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 800.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;
}

void ASBlackholeProjectile::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	GetWorldTimerManager().SetTimer(TimerHandle_DamageTick, this, &ASBlackholeProjectile::ApplyPullDamage, 1.0f, true);
	SetLifeSpan(LifeSpanDuration);
}

void ASBlackholeProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 到达最大飞行距离后停下
	if (MovementComp->IsActive() && FVector::Dist(SpawnLocation, GetActorLocation()) >= MaxRange)
	{
		MovementComp->StopMovementImmediately();
	}

	// 吸引 AI 角色
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Character = *It;
		float Distance = FVector::Dist(GetActorLocation(), Character->GetActorLocation());
		if (Distance > PullRadius) continue;

		FVector PullDir = (GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
		float MoveAmount = FMath::Min(PullStrength * DeltaTime, Distance * 0.5f);
		Character->AddActorWorldOffset(PullDir * MoveAmount, true);
	}

	// 吸引物理模拟物体（爆炸桶、Cube 等），只对 IsSimulatingPhysics() 的组件施力
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor == this || Actor == GetInstigator()) continue;
		if (Actor->IsA<ASAICharacter>()) continue;

		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (!PrimComp || !PrimComp->IsSimulatingPhysics()) continue;

		float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		if (Distance > PullRadius) continue;

		FVector PullDir = (GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
		PrimComp->AddForce(PullDir * PullStrength * PrimComp->GetMass() * 10.0f);
	}
}

void ASBlackholeProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	MovementComp->StopMovementImmediately();
}

void ASBlackholeProjectile::ApplyPullDamage()
{
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Character = *It;
		float Distance = FVector::Dist(GetActorLocation(), Character->GetActorLocation());
		if (Distance > PullRadius) continue;

		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			Character->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (AttributeComp)
		{
			AttributeComp->ApplyHealthChange(GetInstigator(), -DamagePerSecond);
		}
	}
}
