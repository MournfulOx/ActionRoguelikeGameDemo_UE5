// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SAICharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ASAICharacter::ASAICharacter()
{
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");
	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAICharacter::OnPawnSeen);
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
}

void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f && NewHealth > 0.0f)
	{
		
		if (InstigatorActor != this)
		{
			SetTargetActor(InstigatorActor);
		}
		
		// 受击闪光（死后不触发，避免覆盖 DissolveMI 参数）
		GetMesh()->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->GetTimeSeconds());
	}

	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		// 停止 AI 行为
		AAIController* AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			AIC->GetBrainComponent()->StopLogic("Killed");
		}

		// 开启 ragdoll
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName("Ragdoll");
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 开始溶解效果
		if (DissolveMaterial)
		{
			DissolveMI = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
			for (int32 i = 0; i < GetMesh()->GetNumMaterials(); i++)
			{
				GetMesh()->SetMaterial(i, DissolveMI);
			}
			DissolveStartTime = GetWorld()->GetTimeSeconds();
			GetWorldTimerManager().SetTimer(TimerHandle_Dissolve, this, &ASAICharacter::UpdateDissolveMaterial, 0.05f, true);
		}

		SetLifeSpan(DissolveDuration + 0.5f);
	}
}
	

void ASAICharacter::UpdateDissolveMaterial()
{
	if (!DissolveMI) return;

	float Elapsed = GetWorld()->GetTimeSeconds() - DissolveStartTime;
	float Alpha = FMath::Clamp(Elapsed / DissolveDuration, 0.0f, 1.0f);
	DissolveMI->SetScalarParameterValue("DissolveAmount", FMath::Lerp(1.0f, -1.0f, Alpha));

	if (Alpha >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Dissolve);
	}
}
	
void ASAICharacter::SetTargetActor(AActor* NewTarget)
{
		AAIController* AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			AIC->GetBlackboardComponent()->SetValueAsObject("TargetActor", NewTarget);
		}
}	

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
		SetTargetActor(Pawn);
		DrawDebugString(GetWorld(), GetActorLocation(), "PLAYER SPOTTED", nullptr, FColor::White, 4.0f, true);
}
	
