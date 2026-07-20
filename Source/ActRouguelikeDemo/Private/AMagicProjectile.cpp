#include "AMagicProjectile.h"
#include "SAttributeComponent.h"
#include "SGameplayFunctionLibrary.h"
#include "SProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAMagicProjectile::AAMagicProjectile()
{
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAMagicProjectile::OnActorOverlap);
}

void AAMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		if (TryParryReflect(OtherActor))
		{
			return;
		}

		if (OtherActor->IsA<ASProjectileBase>()) return;

		// 防止友伤：发射者和目标是同类型（都是 AI）则跳过
		if (GetInstigator() && OtherActor->IsA(GetInstigator()->GetClass()))
		{
			return;
		}

		USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult);

		Explode();
	}
}

void AAMagicProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bDestroyOnBlockingHit) return;

	if (Other && Other != GetInstigator() && !Other->IsA<ASProjectileBase>())
	{
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			Other->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (!AttributeComp)
		{
			AController* InstigatorController = GetInstigator() ? GetInstigator()->GetInstigatorController() : nullptr;
			UGameplayStatics::ApplyDamage(Other, DamageAmount, InstigatorController, this, nullptr);
			Explode();
		}
	}
}
