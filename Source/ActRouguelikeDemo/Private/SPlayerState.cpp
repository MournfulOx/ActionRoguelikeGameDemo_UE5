// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"

void ASPlayerState::AddCredits(int32 Delta)
{
	if (!ensure(Delta > 0))
	{
		return;
	}

	Credits += Delta;
	OnCreditsChanged.Broadcast(this, Credits, Delta);

	UE_LOG(LogTemp, Log, TEXT("%s: Credits +%i, new total: %i"), *GetName(), Delta, Credits);
}

bool ASPlayerState::RemoveCredits(int32 Delta)
{
	if (!ensure(Delta > 0))
	{
		return false;
	}

	if (Credits < Delta)
	{
		return false;
	}

	Credits -= Delta;
	OnCreditsChanged.Broadcast(this, Credits, -Delta);

	UE_LOG(LogTemp, Log, TEXT("%s: Credits -%i, new total: %i"), *GetName(), Delta, Credits);
	return true;
}
