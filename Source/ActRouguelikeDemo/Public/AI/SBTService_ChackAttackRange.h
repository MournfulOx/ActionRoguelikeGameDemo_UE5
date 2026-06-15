// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "SBTService_ChackAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class ACTROUGUELIKEDEMO_API USBTService_ChackAttackRange : public UBTService
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector AttackRangeKey;
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
