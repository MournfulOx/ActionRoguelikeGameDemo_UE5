#include "AI/SBTTask_HealSelf.h"
#include "AIController.h"
#include "SAttributeComponent.h"

USBTTask_HealSelf::USBTTask_HealSelf()
{
	NodeName = "Heal Self";
}

EBTNodeResult::Type USBTTask_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(AIPawn);
	if (!ensure(AttributeComp)) return EBTNodeResult::Failed;

	AttributeComp->ApplyHealthChange(AIPawn, AttributeComp->GetHealthMax() * HealFraction);
	return EBTNodeResult::Succeeded;
}
