// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmo.h"
#include "TranslationGizmo.generated.h"

/**
 *
 */
UCLASS()
class RUNTIMETRANSFORMER_API ATranslationGizmo : public ABaseGizmo
{
	GENERATED_BODY()

public:

	ATranslationGizmo();

	virtual ETransformationType GetGizmoType() const final { return ETransformationType::TT_Translation; }

	virtual FTransform GetDeltaTransform(const FVector& LookingVector
		, const FVector& RayStartPoint
		, const FVector& RayEndPoint,  ETransformationDomain Domain) override;

	// Returns a Snapped Transform based on how much has been accumulated, the Delta Transform and Snapping Value
	virtual FTransform GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
		, const FTransform& DeltaTransform
		, ETransformationDomain Domain
		, float SnappingValue) const override;

protected:

	// The Hit Box for the XY-Plane Translation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* XY_PlaneBox;

	// The Hit Box for the YZ-Plane Translation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* YZ_PlaneBox;

	// The Hit Box for the	XZ-Plane Translation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* XZ_PlaneBox;

	// The Hit Box for the	XYZ Free Translation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* XYZ_Sphere;
};

