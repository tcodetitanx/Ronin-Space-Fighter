#include "Actors/Asteroid.h"
#include "Components/SphereComponent.h"
#include "ProceduralMeshComponent.h"
#include "Procedural/ProceduralShipMeshBuilder.h"
#include "Components/HealthShieldComponent.h"

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(200.f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();
}

void AAsteroid::InitializeAsteroid(float InRadius, int32 Seed)
{
	CollisionComp->SetSphereRadius(InRadius);

	FRandomStream Rand(Seed);
	RotationSpeed = FRotator(
		Rand.FRandRange(-5.f, 5.f),
		Rand.FRandRange(-5.f, 5.f),
		Rand.FRandRange(-5.f, 5.f)
	);

	FProceduralShipMeshBuilder::BuildAsteroidMesh(MeshComp, InRadius, Seed);
}

void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(RotationSpeed * DeltaTime);
}
