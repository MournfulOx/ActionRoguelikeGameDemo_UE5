// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STargetDummy.generated.h"

UCLASS()
class ACTROUGUELIKEDEMO_API ASTargetDummy : public AActor
{
	GENERATED_BODY()
    
public: 
	ASTargetDummy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	USAttributeComponent* AttributeComp;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);
};