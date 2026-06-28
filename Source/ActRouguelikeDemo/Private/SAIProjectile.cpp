#include "SAIProjectile.h"
#include "SAttributeComponent.h"
#include "SProjectileBase.h"
#include "Components/SphereComponent.h"

ASAIProjectile::ASAIProjectile()
{
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASAIProjectile::OnActorOverlap);
}

void ASAIProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;
	if (OtherActor->IsA<ASProjectileBase>()) return;

	// 防止 AI 互伤
	if (GetInstigator() && OtherActor->IsA(GetInstigator()->GetClass())) return;

	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
		OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));
	if (AttributeComp)
	{
		AttributeComp->ApplyHealthChange(GetInstigator(), -DamageAmount);
	}

	Explode();
}

void ASAIProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Other && Other->IsA<ASProjectileBase>()) return;
	Explode();
}
