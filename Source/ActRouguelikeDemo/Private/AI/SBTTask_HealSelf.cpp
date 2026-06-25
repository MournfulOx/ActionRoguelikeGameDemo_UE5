#include "AI/SBTTask_HealSelf.h"
#include "AIController.h"
#include "SAttributeComponent.h"

EBTNodeResult::Type USBTTask_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
		AIPawn->GetComponentByClass(USAttributeComponent::StaticClass()));
	if (!ensure(AttributeComp)) return EBTNodeResult::Failed;

	// 恢复满血（ApplyHealthChange 内部会 Clamp 到 HealthMax）
	AttributeComp->ApplyHealthChange(AIPawn, AttributeComp->GetHealthMax());
	return EBTNodeResult::Succeeded;
}
