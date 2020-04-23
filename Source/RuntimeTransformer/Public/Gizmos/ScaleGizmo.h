// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmo.h"
#include "ScaleGizmo.generated.h"

/**
 *
 */
UCLASS()
class RUNTIMETRANSFORMER_API AScaleGizmo : public ABaseGizmo
{
	GENERATED_BODY()

public:

	AScaleGizmo();

	virtual ETransformationType GetGizmoType() const final { return ETransformationType::TT_Scale; }

	virtual void UpdateGizmoSpace(TEnumAsByte<ESpaceType> SpaceType);

	//Scale does not support World Space! (Just like UE4's Scale does not!)
	virtual bool SupportsWorldSpace() const { return false; };

	virtual FTransform GetDeltaTransform(const FVector& LookingVector, const FVector& RayStartPoint
		, const FVector& RayEndPoint, TEnumAsByte<ETransformationDomain> Domain) override;

protected:

	//To see how much an Unreal Unit affects Scaling (e.g. how powerful the mouse scales the object!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScalingFactor;

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
