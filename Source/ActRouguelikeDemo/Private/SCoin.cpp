#include "SCoin.h"
#include "SPlayerState.h"

ASCoin::ASCoin()
{
	CreditsAmount = 80;
}

void ASCoin::OnActivated_Implementation(APawn* InstigatorPawn)
{
	if (InstigatorPawn == nullptr)
	{
		return;
	}

	ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>();
	if (PS)
	{
		PS->AddCredits(CreditsAmount);
		HideAndCooldown();
	}
}
