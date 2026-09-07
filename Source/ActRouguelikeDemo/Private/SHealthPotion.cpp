#include "SHealthPotion.h"
#include "SAttributeComponent.h"
#include "SPlayerState.h"

ASHealthPotion::ASHealthPotion()
{
	HealAmount = 50.0f;
	CreditCost = 50;
}

void ASHealthPotion::OnActivated_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
		InstigatorPawn->GetComponentByClass(USAttributeComponent::StaticClass()));

	if (AttributeComp == nullptr || AttributeComp->IsFullHealth())
	{
		return;
	}

	ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>();
	if (PS && PS->RemoveCredits(CreditCost))
	{
		AttributeComp->ApplyHealthChange(InstigatorPawn, HealAmount);
		HideAndCooldown();
	}
}
