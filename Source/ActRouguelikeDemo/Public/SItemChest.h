#pragma once

#include "CoreMinimal.h"
#include "SGameplayInterface.h"
#include "GameFramework/Actor.h"
#include "SItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTROUGUELIKEDEMO_API ASItemChest : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()


public:
	UPROPERTY(EditAnywhere)
	float TargetPitch;
	

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LidMesh;

	virtual void BeginPlay() override;

public:
	void Interact_Implementation(APawn* InstigatorPawn);
	virtual void Tick(float DeltaTime) override;  // 只保留一个
	ASItemChest();  // 只保留一个
};