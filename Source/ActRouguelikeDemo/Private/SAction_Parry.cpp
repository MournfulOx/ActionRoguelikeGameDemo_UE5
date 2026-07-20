// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_Parry.h"

#include "GameFramework/Character.h"

void USAction_Parry::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	ACharacter* Character = Cast<ACharacter>(Instigator);
	if (Character && ParryAnim)
	{
		Character->PlayAnimMontage(ParryAnim);
	}

	FTimerHandle TimerHandle_Parry;
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "ParryDuration_Elapsed", Instigator);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Parry, Delegate, ParryDuration, false);
}

void USAction_Parry::ParryDuration_Elapsed(AActor* Instigator)
{
	StopAction(Instigator);
}
