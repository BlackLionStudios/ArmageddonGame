// Fill out your copyright notice in the Description page of Project Settings.

#include "PointLightShadowSys.h"
#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "Runtime/Engine/Classes/Components/PointLightComponent.h"
#include "ArmageddonCharacter.h"

// Sets default values
APointLightShadowSys::APointLightShadowSys()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	LightIntensity = 3000.0f;
	LightRadius = 1000.0f;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->bVisible = true;
	PointLight->Intensity = LightIntensity;
	PointLight->SetAttenuationRadius(LightRadius);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(PointLight);
	Sphere->SetCollisionProfileName(TEXT("Trigger"));

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &APointLightShadowSys::OnOverlapBegin);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &APointLightShadowSys::OnOverlapEnd);
}

// Called when the game starts or when spawned
void APointLightShadowSys::BeginPlay()
{
	Super::BeginPlay();
	Sphere->SetSphereRadius(PointLight->AttenuationRadius);
}

// Called every frame
void APointLightShadowSys::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Player->IsValidLowLevel())
	{
		// var for hit result
		FHitResult OutHit;

		//gets this actors location and set var for it
		FVector Start = GetActorLocation();
		FVector End = Player->GetActorLocation();
		// other paramaters for collision
		FCollisionQueryParams CollisionParams;

		//if hits something return True otherwise return False
		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Camera, CollisionParams))
		{
			if (OutHit.GetActor() == Player)
			{
				Distance = FMath::Min(Player->GetDistanceTo(this), PointLight->AttenuationRadius);
				Brightness = pow(1 - pow(Distance/(PointLight->AttenuationRadius), 4), 2);
			}
			else
			{
				Brightness = 0.0f;
			}
		}
	}
	else
	{
		Brightness = 0.0f;
	}
}

void APointLightShadowSys::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AArmageddonCharacter::StaticClass()))
	{
		Player = Cast<AArmageddonCharacter>(OtherActor);
		Player->Lights.AddUnique(this);
	}
}

void APointLightShadowSys::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AArmageddonCharacter::StaticClass()))
	{
		Player->Lights.Remove(this);
	}
}
