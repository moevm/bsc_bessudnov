// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ADroneCharacter::ADroneCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DroneCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("DroneCamera"));
	DroneCamera->SetupAttachment(GetRootComponent());

	CommandFileName = FPaths::ProjectDir();
	CommandFileName.Append(TEXT("command.txt"));

	StatusFileName = FPaths::ProjectDir();
	StatusFileName.Append(TEXT("status.txt"));
}

// Called every frame
void ADroneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsExecutingCommand)
	{
		AddMovementInput(CommandVector, 1.0f);
	} else
	{
		AnalyzeScriptCommand(DeltaTime);
	}

	//Tilting Camera Effect
	const FRotator CurrentCameraRotation = DroneCamera->GetComponentRotation();
	
	const FVector NormalizedVelocity = GetVelocity().GetSafeNormal();
	
	const float VelocityForwardDotProduct = FVector::DotProduct(GetActorForwardVector(), NormalizedVelocity);
	const float VelocityRightDotProduct = FVector::DotProduct(GetActorRightVector(), NormalizedVelocity);
	
	FRotator NewCameraRotation = FRotator::ZeroRotator;
	
	NewCameraRotation.Pitch = FMath::FInterpTo(CurrentCameraRotation.Pitch, -VelocityForwardDotProduct * MaxPitchAngle, DeltaTime, 5.0f);
	NewCameraRotation.Roll = FMath::FInterpTo(CurrentCameraRotation.Roll, VelocityRightDotProduct * MaxRollAngle, DeltaTime, 5.0f);
	
	DroneCamera->SetRelativeRotation(NewCameraRotation);

	//Recording
	if (IsRecording)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), FString("Shot"));
		RecorderVelocities.Add(GetVelocity());
		RecordedLocations.Add(DroneCamera->GetComponentLocation() - RecordStartLocation);
		RecordedRotations.Add(UKismetMathLibrary::NormalizedDeltaRotator(DroneCamera->GetComponentRotation(), RecordStartRotation));
		RecordedTimes.Add(RecordingTime);
		RecordingTime += DeltaTime;

		const FVector StartTrace = GetActorLocation();
		const FVector EndTrace = StartTrace + GetActorForwardVector() * 1000000.0f;
		FHitResult HitResult;
		bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility);
		if (IsHit)
		{
			RecordedDistances.Add(HitResult.Distance);
		} else
		{
			RecordedDistances.Add(1000000.0f);
		}
	}
	
}

// Called to bind functionality to input
void ADroneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Movement
	PlayerInputComponent->BindAxis("MoveUp", this, &ADroneCharacter::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADroneCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ADroneCharacter::MoveForward);
	// PlayerInputComponent->BindAxis("TurnRight", this, &ADroneCharacter::AddControllerYawInput);

	//Recording
	PlayerInputComponent->BindAction("Record", IE_Pressed, this, &ADroneCharacter::Record);

	PlayerInputComponent->BindAction("TurnAround", IE_Pressed, this, &ADroneCharacter::TurnAround);
	PlayerInputComponent->BindAction("TurnForward", IE_Pressed, this, &ADroneCharacter::TurnForward);
	PlayerInputComponent->BindAction("TurnBackward", IE_Pressed, this, &ADroneCharacter::TurnBackward);
	PlayerInputComponent->BindAction("HalfTurnLeft", IE_Pressed, this, &ADroneCharacter::HalfTurnLeft);
	PlayerInputComponent->BindAction("HalfTurnRight", IE_Pressed, this, &ADroneCharacter::HalfTurnRight);
}

void ADroneCharacter::BeginPlay()
{
	Super::BeginPlay();
	ClearScreenShotsDirectory();
	RecordStartLocation = DroneCamera->GetComponentLocation();
	RecordStartRotation = DroneCamera->GetComponentRotation();
}

void ADroneCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void ADroneCharacter::MoveUp(float Value)
{
	AddMovementInput(GetActorUpVector(), Value);
}

void ADroneCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void ADroneCharacter::StartSectionRecord(int SectionIndex)
{
	
}

void ADroneCharacter::StopSectionRecord()
{
	
}

FString ADroneCharacter::ReadCommandFromFile()
{	
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	FString FileContent;
	const FString CommandString = "-"; 

	if (FileManager.FileExists(*CommandFileName))
	{
		FFileHelper::LoadFileToString(FileContent, *CommandFileName);
	}

	if (FileManager.FileExists(*CommandFileName))
	{
		FFileHelper::SaveStringToFile(CommandString, *CommandFileName);
	}

	return FileContent;
}

void ADroneCharacter::AnalyzeScriptCommand(const float& DeltaTime)
{
	FString CommandFull = ReadCommandFromFile();
	FString Command;
	FString Value;
	CommandFull.Split(" ", &Command, &Value);

	const float CommandTime = FCString::Atof(*Value);
	if (Command == "MoveLeft")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorRightVector() * (-1.0f);
		GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}

	if (Command == "MoveRight")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorRightVector();
        GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}

	if (Command == "MoveUp")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorUpVector();
		GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}

	if (Command == "MoveDown")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorUpVector() * (-1.0f);
		GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}

	if (Command == "MoveForward")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorForwardVector();
		GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}
	
	if (Command == "MoveBackward")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = true;
		CommandVector = GetActorForwardVector() * (-1.0f);
		GetWorld()->GetTimerManager().SetTimer(TH_Command, this, &ADroneCharacter::CommandEnd, CommandTime, false);
	}

	if (Command == "TurnRight")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = false;
		CommandFloat = 1.0f;
		TargetAngleDelta = CommandTime;
		TurnStartRotation = GetActorRotation();
		GetWorld()->GetTimerManager().SetTimer(TH_Turn, this, &ADroneCharacter::TurnDrone, 1.5f, false);
	}

	if (Command == "TurnLeft")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = false;
		CommandFloat = -1.0f;
		TargetAngleDelta = CommandTime;
		TurnStartRotation = GetActorRotation();
		GetWorld()->GetTimerManager().SetTimer(TH_Turn, this, &ADroneCharacter::TurnDrone, 1.5f, false);
	}

	if (Command == "RotateForward")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = false;
		CommandFloat = 1.0f;
		TargetAngleDelta = CommandTime;
		TurnStartRotation = GetActorRotation();
		GetWorld()->GetTimerManager().SetTimer(TH_Turn, this, &ADroneCharacter::RotateDrone, 1.5f, false);
	}

	if (Command == "RotateBackward")
	{
		WriteStatus("Bad");
		IsExecutingCommand = true;
		IsRecording = false;
		CommandFloat = -1.0f;
		TargetAngleDelta = CommandTime;
		TurnStartRotation = GetActorRotation();
		GetWorld()->GetTimerManager().SetTimer(TH_Turn, this, &ADroneCharacter::RotateDrone, 1.5f, false);
	}
}

void ADroneCharacter::WriteStatus(FString Status)
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (FileManager.FileExists(*StatusFileName))
	{
		FFileHelper::SaveStringToFile(Status, *StatusFileName);
	}
}

void ADroneCharacter::CommandEnd()
{
	GetWorld()->GetTimerManager().ClearTimer(TH_Command);
	GetWorld()->GetTimerManager().ClearTimer(TH_Turn);
	IsExecutingCommand = false;
	IsRecording = false;
	CommandVector = FVector::ZeroVector;
	CommandFloat = 0.0f;
	WriteInfoToFiles();
	WriteStatus("Ok");
}

void ADroneCharacter::TurnAround()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	AddControllerYawInput(180.0f / Con->InputYawScale);
}

void ADroneCharacter::TurnForward()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	AddControllerPitchInput(45.0f / Con->InputPitchScale);
}

void ADroneCharacter::TurnBackward()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	AddControllerPitchInput(-45.0f / Con->InputPitchScale);
}

void ADroneCharacter::TurnDrone()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	GetWorld()->GetTimerManager().ClearTimer(TH_Turn);
	AddControllerYawInput(CommandFloat * TargetAngleDelta / Con->InputYawScale);
	CommandEnd();
}

void ADroneCharacter::RotateDrone()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	GetWorld()->GetTimerManager().ClearTimer(TH_Turn);
	AddControllerPitchInput(CommandFloat * TargetAngleDelta / Con->InputPitchScale);
	CommandEnd();
}

void ADroneCharacter::HalfTurnLeft()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	AddControllerYawInput(45.0f / Con->InputYawScale);
}

void ADroneCharacter::HalfTurnRight()
{
	APlayerController *Con = Cast<APlayerController>(GetController());
	AddControllerYawInput(-45.0f / Con->InputYawScale);	
}


void ADroneCharacter::Record()
{
	RecordingTime = 0.0f;
	RecordStartLocation = DroneCamera->GetComponentLocation();
	RecordStartRotation = DroneCamera->GetComponentRotation();

	IsRecording = !IsRecording;
	if (!IsRecording)
	{
		WriteInfoToFiles();
	}
}

void ADroneCharacter::WriteInfoToFiles()
{
	FString FileNameTrajectory = FPaths::ProjectDir();
	FileNameTrajectory.Append(TEXT("trajectory.txt"));

	FString FileNameTimes = FPaths::ProjectDir();
	FileNameTimes.Append(TEXT("times.txt"));

	FString FileNameVelocities = FPaths::ProjectDir();
	FileNameVelocities.Append(TEXT("velocities.txt"));

	FString FileNameDistances = FPaths::ProjectDir();
	FileNameDistances.Append(TEXT("distances.txt"));

	// We will use this FileManager to deal with the file.
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> TimesStrings;
	TArray<FString> TrajectoryStrings;
	TArray<FString> VelocitiesStrings;
	TArray<FString> DistancesStrings;

	for (auto &TimeStep : RecordedTimes)
	{
		TimesStrings.Add(FString::SanitizeFloat(TimeStep));
	}

	for (auto &Distance : RecordedDistances)
	{
		DistancesStrings.Add(FString::SanitizeFloat(Distance));
	}

	for (int i = 0; i < RecordedLocations.Num(); i++)
	{
		FString TrajectoryString = RecordedLocations[i].ToString();
		TrajectoryString.Append(" ");
		TrajectoryString.Append(RecordedRotations[i].ToString());
		TrajectoryStrings.Add(TrajectoryString);
		VelocitiesStrings.Add(RecorderVelocities[i].ToString());
	}

	// Always first check if the file that you want to manipulate exist.
	if (FileManager.FileExists(*FileNameTimes))
	{
		FFileHelper::SaveStringArrayToFile(TimesStrings, *FileNameTimes);
	}

	if (FileManager.FileExists(*FileNameTrajectory))
	{
		FFileHelper::SaveStringArrayToFile(TrajectoryStrings, *FileNameTrajectory);
	}

	if (FileManager.FileExists(*FileNameVelocities))
	{
		FFileHelper::SaveStringArrayToFile(VelocitiesStrings, *FileNameVelocities);
	}

	if (FileManager.FileExists(*FileNameDistances))
	{
		FFileHelper::SaveStringArrayToFile(DistancesStrings, *FileNameDistances);
	}
}

void ADroneCharacter::ClearScreenShotsDirectory() const
{
	FString ScreenshotsDirectoryName = FPaths::ProjectSavedDir();
	ScreenshotsDirectoryName.Append(TEXT("Screenshots"));

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	FileManager.DeleteDirectoryRecursively(*ScreenshotsDirectoryName);
}

