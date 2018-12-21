// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointLightShadowSys.generated.h"

UCLASS()
class ARMAGEDDON_API APointLightShadowSys : public AActor
{
	GENERATED_BODY()

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, Category = "PointLightShadow")
	class UPointLightComponent* PointLight;

	UPROPERTY(VisibleAnywhere, Category = "PointLightShadow")
	class USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, Category = "PointLightShadow")
	float LightIntensity;

	UPROPERTY(VisibleAnywhere, Category = "PointLightShadow")
	float LightRadius;

public:	
	// Sets default values for this actor's properties
	APointLightShadowSys();

	class AArmageddonCharacter* Player;

	float Brightness;

	float Distance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
