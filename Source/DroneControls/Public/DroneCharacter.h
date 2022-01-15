// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "GameFramework/Character.h"
#include "DroneCharacter.generated.h"

UCLASS()
class DRONECONTROLS_API ADroneCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADroneCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

protected:	
	//Movement controls
	void MoveForward(float Value);
	void MoveUp(float Value);
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable)
	void StartSectionRecord(int SectionIndex);

	UFUNCTION(BlueprintCallable)
	void StopSectionRecord();

	//Recording Stuff
	UFUNCTION(BlueprintCallable)
	void Record();

public:
	//Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone Components")
	UCineCameraComponent *DroneCamera;

protected:

	//Functions for dealing with script control
	UFUNCTION(BlueprintCallable)
	FString ReadCommandFromFile();

	UFUNCTION(BlueprintCallable)
	void AnalyzeScriptCommand(const float& DeltaTime);

	UFUNCTION(BlueprintCallable)
	void WriteStatus(FString Status);

	UPROPERTY(EditDefaultsOnly)
	float MaxPitchAngle = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	float MaxRollAngle = 5.0f;
	
private:
	UFUNCTION()
	void CommandEnd();

	UFUNCTION()
	void TurnAround();

	UFUNCTION()
	void TurnForward();

	UFUNCTION()
	void TurnBackward();

	UFUNCTION()
	void TurnDrone();

	UFUNCTION()
    void RotateDrone();

	UFUNCTION()
	void HalfTurnLeft();

	UFUNCTION()
	void HalfTurnRight();

	UPROPERTY()
	float TurnTick = 0.1f;

	UPROPERTY()
	float TargetAngleDelta;

	UPROPERTY()
	FRotator TurnStartRotation;

	UPROPERTY()
	FString CommandFileName;

	UPROPERTY()
	FString StatusFileName;

	UPROPERTY()
	FTimerHandle TH_Command;

	UPROPERTY()
	FTimerHandle TH_Turn;

	UPROPERTY()
	FVector CommandVector;

	UPROPERTY()
	float CommandFloat;

	UPROPERTY()
	bool IsExecutingCommand = false;
	

	UFUNCTION()
	void WriteInfoToFiles();

	UFUNCTION()
	void ClearScreenShotsDirectory() const;

	UPROPERTY()
	bool IsRecording = false;
	
	UPROPERTY()
	FVector RecordStartLocation;

	UPROPERTY()
	FRotator RecordStartRotation;
	
	UPROPERTY()
	float RecordingTime;

	UPROPERTY()
	TArray<FVector> RecordedLocations;

	UPROPERTY()
	TArray<FRotator> RecordedRotations;

	UPROPERTY()
	TArray<float> RecordedTimes;

	UPROPERTY()
	TArray<float> RecordedDistances;

	UPROPERTY()
	TArray<FVector> RecorderVelocities;

	

};
