// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SActionComponent.h"
#include "GameFramework/Character.h"
#include "SAICharacter.generated.h"

class UPawnSensingComponent;
class USAttributeComponent;
class UUserWidget;
class USWorldUserWidget;
class USActionComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API ASAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASAICharacter();

protected:
	
	USWorldUserWidget* ActiveHealthBar;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;
	
	void SetTargetActor(AActor* NewTarget);

	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USActionComponent* ActionComp;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);
};
