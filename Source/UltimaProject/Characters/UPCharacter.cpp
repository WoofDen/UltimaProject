// Fill out your copyright notice in the Description page of Project Settings.

#include "UPCharacter.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"
#include "UltimaProject/Items/Containers/PlayerInventory/InventoryComponent.h"

AUPPlayerController* AUPCharacter::GetPlayerController()
{
	if (PlayerController == nullptr)
	{
		// todo better way to do this
		AController* PawnController = const_cast<AController*>(GetController());
		PlayerController = Cast<AUPPlayerController>(PawnController);
		check(PlayerController);
	}

	return PlayerController;
}

// Sets default values
AUPCharacter::AUPCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetArrowComponent()->bHiddenInGame = false;
	GetArrowComponent()->SetVisibility(true);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// setup camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");

	SpringArmComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CameraComponent->AttachToComponent(SpringArmComponent, FAttachmentTransformRules::KeepRelativeTransform,
	                                   SpringArmComponent->GetAttachSocketName());

	SpringArmComponent->TargetArmLength = 900.f;
	FRotator ArmRotator;
	ArmRotator.Pitch = -40.f;
	SpringArmComponent->SetWorldRotation(ArmRotator);

	// gas
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");

	// skills
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>("SkillSystemComponent");

	// Inv
	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
}

// Called when the game starts or when spawned
void AUPCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AUPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AUPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AUPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AUPCharacter, Inventory, COND_OwnerOnly);
}

void AUPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(AbilitySystemComponent);
	AbilitySystemComponent->AddSet<UUPBaseAttributeSet>();
}
