#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SBTTask_HealSelf.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API USBTTask_HealSelf : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USBTTask_HealSelf();

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	float HealFraction = 0.5f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
