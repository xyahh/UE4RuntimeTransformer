// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmo.h"
#include "RotationGizmo.generated.h"

/**
 *
 */
UCLASS()
class RUNTIMETRANSFORMER_API ARotationGizmo : public ABaseGizmo
{
	GENERATED_BODY()

public:

	ARotationGizmo();

	virtual ETransformationType GetGizmoType() const final { return ETransformationType::TT_Rotation; }

protected:

	//Rotation has a special way of Handling the Scene Scaling and that is, that its AXis need to face the Camera as well!
	virtual FVector CalculateGizmoSceneScale(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView) override;


	virtual FTransform GetDeltaTransform(const FVector& LookingVector, const FVector& RayStartPoint
		, const FVector& RayEndPoint,  TEnumAsByte<ETransformationDomain> Domain) override;

private:

	FVector PreviousRotationViewScale;
};
