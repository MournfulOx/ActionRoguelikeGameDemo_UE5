// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAICharacter.generated.h"

class UPawnSensingComponent;
class USAttributeComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API ASAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASAICharacter();

protected:

	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USAttributeComponent* AttributeComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* DissolveMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DissolveMI;

	FTimerHandle TimerHandle_Dissolve;
	float DissolveStartTime;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float DissolveDuration = 3.0f;

	void UpdateDissolveMaterial();

	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);
};
