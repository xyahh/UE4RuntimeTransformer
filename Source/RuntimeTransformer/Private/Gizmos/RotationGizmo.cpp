// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#include "Gizmos/RotationGizmo.h"

ARotationGizmo::ARotationGizmo()
{
	PreviousRotationViewScale = FVector::OneVector;
}

FVector ARotationGizmo::CalculateGizmoSceneScale(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView)
{
	FVector calculatedScale = Super::CalculateGizmoSceneScale(ReferenceLocation, ReferenceLookDirection, FieldOfView);
	FVector currentRotationViewScale = PreviousRotationViewScale;
	
	bool bInProgress = GetTransformProgressState();
	if (!bInProgress)
	{
		FVector deltaLocation = ReferenceLocation - GetActorLocation();
	
		currentRotationViewScale = FVector(
			(FVector::DotProduct(GetActorForwardVector(), deltaLocation)	>= 0) ? 1.f : -1.f,
			(FVector::DotProduct(GetActorRightVector()	, deltaLocation)	>= 0) ? 1.f : -1.f,
			(FVector::DotProduct(GetActorUpVector()		, deltaLocation)	>= 0) ? 1.f : -1.f
		);

		PreviousRotationViewScale = currentRotationViewScale;
	}
	
	
	calculatedScale *= currentRotationViewScale;

	



	return calculatedScale;
}

FTransform ARotationGizmo::GetDeltaTransform(const FVector& LookingVector, const FVector& RayStartPoint
	, const FVector& RayEndPoint,  TEnumAsByte<ETransformationDomain> Domain)
{
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector);

	if (AreRaysValid())
	{
		FVector planeNormal = FVector(1.f, 0.f, 0.f);

		switch (Domain)
		{
		case ETransformationDomain::TD_X_Axis:planeNormal = GetActorForwardVector();break;
		case ETransformationDomain::TD_Y_Axis: planeNormal = GetActorRightVector(); break;
		case ETransformationDomain::TD_Z_Axis: planeNormal = GetActorUpVector(); break;
		}

		const FVector gizmoLocation = GetActorLocation();

		FPlane plane;
		plane.X = planeNormal.X;
		plane.Y = planeNormal.Y;
		plane.Z = planeNormal.Z;
		plane.W = FVector::PointPlaneDist(gizmoLocation, FVector::ZeroVector, planeNormal);

		FVector deltaLocation = FMath::LinePlaneIntersection(RayStartPoint, RayEndPoint, plane) - gizmoLocation;
		FVector prevDeltaLocation = FMath::LinePlaneIntersection(PreviousRayStartPoint, PreviousRayEndPoint, plane) - gizmoLocation;

		//determining direction of Angle
		float factor = (FVector::DotProduct(FVector::CrossProduct(deltaLocation, prevDeltaLocation), planeNormal)) >= 0.f ?
			-1.f : 1.f;
		
		FVector diffOfDeltas = deltaLocation - prevDeltaLocation;
		
		deltaLocation.Normalize();
		prevDeltaLocation.Normalize();

		float angle = diffOfDeltas.Size() < 0.01f ? 0.f : FMath::Acos(FVector::DotProduct(deltaLocation, prevDeltaLocation));

		angle *= factor;
		FQuat rotQuat = FQuat(planeNormal, angle);
		deltaTransform.SetRotation(rotQuat);

	}

	UpdateRays(RayStartPoint, RayEndPoint);

	return deltaTransform;
}

FTransform ARotationGizmo::GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
	, const FTransform& DeltaTransform
	, TEnumAsByte<ETransformationDomain> Domain
	, float SnappingValue) const
{
	if (SnappingValue == 0.f) return DeltaTransform;

	FTransform result = DeltaTransform;

	FRotator addedRotation = outCurrentAccumulatedTransform.GetRotation().Rotator() 
		+ DeltaTransform.GetRotation().Rotator();
	
	FRotator snappedRotation = addedRotation.GridSnap(FRotator(SnappingValue));
	result.SetRotation(snappedRotation.Quaternion());
	outCurrentAccumulatedTransform.SetRotation((addedRotation - snappedRotation).Quaternion());

	return result;
}
