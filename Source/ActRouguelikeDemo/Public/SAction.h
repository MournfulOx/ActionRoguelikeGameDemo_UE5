// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SActionComponent.h"
#include "UObject/NoExportTypes.h"
#include "SAction.generated.h"

class UWorld;

/**
 * 
 */
UCLASS(Blueprintable)
class ACTROUGUELIKEDEMO_API USAction : public UObject
{
	GENERATED_BODY()
	
	
protected:
	UFUNCTION(BlueprintCallable, Category="Action")
	USActionComponent* GetOwningComponent() const;
	
	UPROPERTY(EditDefaultsOnly, Category="Tags")
	FGameplayTagContainer GrantsTage;
	
	UPROPERTY(EditDefaultsOnly, Category="Tags")
	FGameplayTagContainer BlockedTags;

	bool bIsRunning;
public:
	
	UFUNCTION(BlueprintCallable, Category="Action")
	bool IsRunning() const;
	
	UFUNCTION(BlueprintNativeEvent, Category="Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category= "Action")
	void StartAction(AActor* Instigator);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category= "Action")
	void StopAction(AActor* Instigator);
	
	UPROPERTY(EditDefaultsOnly, Category="Action")
	FName ActionName;
	
	UWorld* GetWorld() const override;

};