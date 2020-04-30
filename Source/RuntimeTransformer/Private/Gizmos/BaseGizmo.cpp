// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "Gizmos/BaseGizmo.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

// Sets default values
ABaseGizmo::ABaseGizmo()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	ScalingScene = CreateDefaultSubobject<USceneComponent>(TEXT("ScalingScene"));
	ScalingScene->SetupAttachment(RootScene);

	X_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("X Axis Box"));
	Y_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Y Axis Box"));
	Z_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Z Axis Box"));

	X_AxisBox->SetupAttachment(ScalingScene);
	Y_AxisBox->SetupAttachment(ScalingScene);
	Z_AxisBox->SetupAttachment(ScalingScene);

	RegisterDomainComponent(X_AxisBox, ETransformationDomain::TD_X_Axis);
	RegisterDomainComponent(Y_AxisBox, ETransformationDomain::TD_Y_Axis);
	RegisterDomainComponent(Z_AxisBox, ETransformationDomain::TD_Z_Axis);

	GizmoSceneScaleFactor = 0.1f;
	CameraArcRadius = 150.f;

	PreviousRayStartPoint = FVector::ZeroVector;
	PreviousRayEndPoint = FVector::ZeroVector;

	bTransformInProgress = false;
	bIsPrevRayValid = false;
}

void ABaseGizmo::UpdateGizmoSpace(ESpaceType SpaceType)
{
	switch (SpaceType)
	{
	case ESpaceType::ST_Local:
		SetActorRelativeRotation(FQuat(EForceInit::ForceInit));
		break;
	case ESpaceType::ST_World:
		SetActorRotation(FQuat(EForceInit::ForceInit), ETeleportType::TeleportPhysics);
		break;
	}
}

//Base Gizmo does not affect anything and returns No Delta Transform.
// This func is overriden by each Transform Gizmo

FTransform ABaseGizmo::GetDeltaTransform(const FVector& LookingVector
	, const FVector& RayStartPoint, const FVector& RayEndPoint
	,  ETransformationDomain Domain)
{
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector);
	return deltaTransform;
}

void ABaseGizmo::ScaleGizmoScene(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView)
{
	FVector Scale = CalculateGizmoSceneScale(ReferenceLocation, ReferenceLookDirection, FieldOfView);
	//UE_LOG(LogRuntimeTransformer, Warning, TEXT("Scale: %s"), *Scale.ToString());
	if (ScalingScene)
		ScalingScene->SetWorldScale3D(Scale);
}

FTransform ABaseGizmo::GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
	, const FTransform& DeltaTransform
	, ETransformationDomain Domain
	, float SnappingValue) const
{
	return DeltaTransform;
}

ETransformationDomain ABaseGizmo::GetTransformationDomain(USceneComponent* ComponentHit) const
{
	if (!ComponentHit) return ETransformationDomain::TD_None;

	if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(ComponentHit))
	{
		if (const ETransformationDomain* pDomain = DomainMap.Find(ShapeComponent))
			return *pDomain;
	}
	else
		UE_LOG(LogRuntimeTransformer, Warning, TEXT("Failed to Get Domain! Component Hit is not a Shape Component. %s"), *ComponentHit->GetName());

	return ETransformationDomain::TD_None;
}

FVector ABaseGizmo::CalculateGizmoSceneScale(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView)
{
	FVector deltaLocation = (GetActorLocation() - ReferenceLocation);
	float distance = deltaLocation.ProjectOnTo(ReferenceLookDirection).Size();
	float scaleView = (distance * FMath::Sin(FMath::DegreesToRadians(FieldOfView))) / CameraArcRadius;
	scaleView *= GizmoSceneScaleFactor;
	return FVector(scaleView);
}

bool ABaseGizmo::AreRaysValid() const
{
	return bIsPrevRayValid;
}

void ABaseGizmo::UpdateRays(const FVector& RayStart, const FVector& RayEnd)
{
	PreviousRayStartPoint = RayStart;
	PreviousRayEndPoint = RayEnd;
	bIsPrevRayValid = true;
}

void ABaseGizmo::RegisterDomainComponent(USceneComponent* Component
	, ETransformationDomain Domain)
{
	if (!Component) return;

	if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(Component))
		DomainMap.Add(ShapeComponent, Domain);
	else
		UE_LOG(LogRuntimeTransformer, Warning, TEXT("Failed to Register Component! Component is not a Shape Component %s"), *Component->GetName());
}

void ABaseGizmo::SetTransformProgressState(bool bInProgress
	, ETransformationDomain CurrentDomain)
{
	if (bInProgress != bTransformInProgress)
	{
		bIsPrevRayValid = false; //set this so that we don't get an invalid delta value
		bTransformInProgress = bInProgress;
		OnGizmoStateChange.Broadcast(GetGizmoType(), bTransformInProgress, CurrentDomain);
	}
}
