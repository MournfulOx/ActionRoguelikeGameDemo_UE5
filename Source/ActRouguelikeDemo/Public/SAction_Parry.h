// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SAction.h"
#include "SAction_Parry.generated.h"

class UAnimMontage;

/**
 *
 */
UCLASS()
class ACTROUGUELIKEDEMO_API USAction_Parry : public USAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Parry")
	float ParryDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Parry")
	UAnimMontage* ParryAnim;

	UFUNCTION()
	void ParryDuration_Elapsed(AActor* Instigator);

public:
	virtual void StartAction_Implementation(AActor* Instigator) override;
};
