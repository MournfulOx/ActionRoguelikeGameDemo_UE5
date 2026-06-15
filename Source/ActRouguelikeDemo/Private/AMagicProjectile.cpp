#include "AMagicProjectile.h"
#include "SAttributeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

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
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (AttributeComp)
		{
			AttributeComp->ApplyHealthChange(GetInstigator(), -20.0f);
		}

		Explode();
	}
}

void AAMagicProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bDestroyOnBlockingHit) return;

	if (Other && Other != GetInstigator())
	{
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
			Other->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (!AttributeComp)
		{
			AController* InstigatorController = GetInstigator() ? GetInstigator()->GetInstigatorController() : nullptr;
			UGameplayStatics::ApplyDamage(Other, 20.0f, InstigatorController, this, nullptr);
			Explode();
		}
	}
}
