#include "SPowerupActor.h"
#include "Components/SphereComponent.h"

ASPowerupActor::ASPowerupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("OverlapAllDynamic");
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;
}

void ASPowerupActor::Interact_Implementation(APawn* InstigatorPawn)
{
	OnActivated(InstigatorPawn);
}

void ASPowerupActor::OnActivated_Implementation(APawn* InstigatorPawn)
{
	// 子类实现具体逻辑
}

void ASPowerupActor::HideAndCooldown()
{
	MeshComp->SetVisibility(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().SetTimer(TimerHandle_Cooldown, this, &ASPowerupActor::RestorePowerup, CooldownDuration);
}

void ASPowerupActor::RestorePowerup()
{
	MeshComp->SetVisibility(true);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
