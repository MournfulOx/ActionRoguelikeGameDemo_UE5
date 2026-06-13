#include "SHealthPotion.h"
#include "SAttributeComponent.h"

ASHealthPotion::ASHealthPotion()
{
	HealAmount = 50.0f;
}

void ASHealthPotion::OnActivated_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
		InstigatorPawn->GetComponentByClass(USAttributeComponent::StaticClass()));

	if (AttributeComp && !AttributeComp->IsFullHealth())
	{
		AttributeComp->ApplyHealthChange(InstigatorPawn, HealAmount);
		HideAndCooldown();
	}
}
