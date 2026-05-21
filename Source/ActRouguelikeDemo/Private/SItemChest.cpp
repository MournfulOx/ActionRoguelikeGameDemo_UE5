#include "SItemChest.h"
#include "Components/StaticMeshComponent.h"

ASItemChest::ASItemChest()
{
	PrimaryActorTick.bCanEverTick = true;  // 只保留一个

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMesh");
	RootComponent = BaseMesh;

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>("LidMesh");
	LidMesh->SetupAttachment(BaseMesh);
	
	TargetPitch = 110;
}

void ASItemChest::Interact_Implementation(APawn* InstigatorPawn)
{
	LidMesh->SetRelativeRotation(FRotator(TargetPitch, 0.f, 0.f));
}

void ASItemChest::BeginPlay()
{
	Super::BeginPlay();
}

void ASItemChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Tick里不应该一直旋转，暂时留空
}
