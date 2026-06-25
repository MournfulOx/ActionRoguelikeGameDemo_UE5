#include "AI/SBTService_CheckHealth.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SAttributeComponent.h"

void USBTService_CheckHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!ensure(BBComp)) return;

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!AIPawn) return;

	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(
		AIPawn->GetComponentByClass(USAttributeComponent::StaticClass()));
	if (!ensure(AttributeComp)) return;

	bool bLowHealth = (AttributeComp->GetHealth() / AttributeComp->GetHealthMax()) <= LowHealthFraction;
	BBComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
}
