// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ArmageddonCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "PointLightShadowSys.h"

//////////////////////////////////////////////////////////////////////////
// AArmageddonCharacter

AArmageddonCharacter::AArmageddonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AArmageddonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("CrouchButton", IE_Pressed, this, &AArmageddonCharacter::DoCrouch);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AArmageddonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AArmageddonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AArmageddonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AArmageddonCharacter::LookUpAtRate);
}


void AArmageddonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AArmageddonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AArmageddonCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AArmageddonCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AArmageddonCharacter::BeginPlay()
{
	//start function on begining
	Super::BeginPlay();

	//get directional light objects in scene
	for (TObjectIterator<UDirectionalLightComponent> Itr; Itr; ++Itr)
	{
		// set Sun variable to directional light object
		Sun = *Itr;
	}
}

// Shadow system to track is character in shadow or not
void AArmageddonCharacter::ShadowSystem()
{	
	// var for hit result
	FHitResult OutHit;

	//gets this actors location and set var for it
	FVector PlayerLocation = GetActorLocation();
	FVector PlayerHeadLocation = GetActorLocation() + FVector(0.f,0.0f,76.0f);
	//gets lights direction vector
	FVector ForwardVector = Cast<UDirectionalLightComponent>(Sun)->GetForwardVector();
	FVector End = (ForwardVector * -100000.f) + PlayerLocation;
	// other paramaters for collision
	FCollisionQueryParams CollisionParams;

	//if hits something return True otherwise return False
	if (GetWorld()->LineTraceSingleByChannel(OutHit, PlayerLocation, End, ECC_Visibility, CollisionParams)&& GetWorld()->LineTraceSingleByChannel(OutHit, PlayerHeadLocation, End, ECC_Visibility, CollisionParams))
	{
		Visibility = 0.0f;
	}
	else
	{
		Visibility = 1.0f;
	}

	for (int32 i = 0; i != Lights.Num(); ++i)
	{
		Visibility = (Lights[i]->Brightness) + Visibility;
	}

	Visibility = FMath::Clamp(Visibility, 0.0f, 1.0f);
}

void AArmageddonCharacter::Tick(float DeltaTime)
{
	//function works every Tick(few frames)
	Super::Tick(DeltaTime);

	ShadowSystem();
	//prints debug text in corner
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-10, 1.f, FColor::Yellow, FString::Printf(TEXT("Visibility: %d"), int(Visibility * 100)));
}

void AArmageddonCharacter::DoCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else if(CanCrouch())
	{
		Crouch();
	}
}