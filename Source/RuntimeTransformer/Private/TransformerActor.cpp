// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "TransformerActor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

/* Gizmos */
#include "Gizmos/BaseGizmo.h"
#include "Gizmos/TranslationGizmo.h"
#include "Gizmos/RotationGizmo.h"
#include "Gizmos/ScaleGizmo.h"

/* Interface */
#include "FocusableObject.h"


// Sets default values
ATransformerActor::ATransformerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GizmoPlacement = EGizmoPlacement::GP_OnLastSelection;
	CurrentTransformation = ETransformationType::TT_Translation;
	CurrentDomain = ETransformationDomain::TD_None;
	CurrentSpaceType = ESpaceType::ST_World;
	TranslationGizmoClass	= ATranslationGizmo::StaticClass();
	RotationGizmoClass		= ARotationGizmo::StaticClass();
	ScaleGizmoClass			= AScaleGizmo::StaticClass();

	bTransformUFocusableObjects = true;
	bRotateOnLocalAxis = false;
	bForceMobility = false;
	bToggleSelectedInMultiSelection = true;
	
}

void ATransformerActor::BeginPlay()
{
	Super::BeginPlay();
	ResetAccumulatedTransform();
	SetTransformationType(CurrentTransformation);
	SetSpaceType(CurrentSpaceType);
}

void ATransformerActor::SetPlayerController(APlayerController* Controller)
{
	PlayerController = Controller;
}

void ATransformerActor::SetSpaceType(TEnumAsByte<ESpaceType> Type)
{
	CurrentSpaceType = Type;
	SetGizmo();
}

TEnumAsByte<ETransformationDomain> ATransformerActor::GetCurrentDomain(bool& TransformInProgress) const
{
	TransformInProgress = (CurrentDomain != ETransformationDomain::TD_None);
	return CurrentDomain;
}

void ATransformerActor::ClearDomain()
{
	CurrentDomain = ETransformationDomain::TD_None;

	//Clear the Accumulated tranform when we stop Transforming
	ResetAccumulatedTransform();

	if (Gizmo.IsValid()) 
		Gizmo->SetTransformProgressState(false, CurrentDomain);
}

UClass* ATransformerActor::GetGizmoClass(TEnumAsByte<ETransformationType> TransformationType) const /* private */
{
	//Assign correct Gizmo Class depending on given Transformation
	switch (CurrentTransformation)
	{
	case ETransformationType::TT_Translation:	return TranslationGizmoClass;
	case ETransformationType::TT_Rotation:		return RotationGizmoClass;
	case ETransformationType::TT_Scale:			return ScaleGizmoClass;
	default:									return nullptr;
	}
}

void ATransformerActor::ResetAccumulatedTransform()
{
	AccumulatedDeltaTransform = FTransform();
	AccumulatedDeltaTransform.SetScale3D(FVector::ZeroVector); //Set Scale to 0 since def makes it 1.f, 1.f, 1.f
}

bool ATransformerActor::MouseTraceByObjectTypes(float TraceDistance, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (PlayerController)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByObjectTypes(worldLocation, worldLocation + worldDirection * TraceDistance
				, CollisionChannels, IgnoredActors, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

bool ATransformerActor::MouseTraceByChannel(float TraceDistance, TEnumAsByte<ECollisionChannel> TraceChannel, TArray<AActor*> IgnoredActors
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (PlayerController)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByChannel(worldLocation, worldLocation + worldDirection * TraceDistance
				, TraceChannel, IgnoredActors, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

bool ATransformerActor::MouseTraceByProfile(float TraceDistance, const FName& ProfileName, TArray<AActor*> IgnoredActors
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (PlayerController)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByProfile(worldLocation, worldLocation + worldDirection * TraceDistance
				, ProfileName, IgnoredActors, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

bool ATransformerActor::TraceByObjectTypes(const FVector& StartLocation, const FVector& EndLocation
	, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionObjectQueryParams CollisionObjectQueryParams;
		FCollisionQueryParams CollisionQueryParams;

		//Add All Given Collisions to the Array
		for (auto& cc : CollisionChannels)
			CollisionObjectQueryParams.AddObjectTypesToQuery(cc);

		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByObjectType(OutHits, StartLocation, EndLocation
			, CollisionObjectQueryParams, CollisionQueryParams))
		{
			return HandleTracedObjects(OutHits, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

bool ATransformerActor::TraceByChannel(const FVector& StartLocation, const FVector& EndLocation
	, TEnumAsByte<ECollisionChannel> TraceChannel
	, TArray<AActor*> IgnoredActors
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation
			, TraceChannel, CollisionQueryParams))
		{
			return HandleTracedObjects(OutHits, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

bool ATransformerActor::TraceByProfile(const FVector& StartLocation, const FVector& EndLocation
	, const FName& ProfileName, TArray<AActor*> IgnoredActors
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByProfile(OutHits, StartLocation, EndLocation
			, ProfileName, CollisionQueryParams))
		{
			return HandleTracedObjects(OutHits, bClearPreviousSelections, bTraceComponent);
		}
	}
	return false;
}

void ATransformerActor::Tick(float DeltaSeconds)
{
	if (!Gizmo.IsValid()) return;

	if (PlayerController)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->PlayerCameraManager)
		{
			if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
				UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector(), worldLocation, worldDirection);

			Gizmo->ScaleGizmoScene(PlayerController->PlayerCameraManager->GetCameraLocation()
				, PlayerController->PlayerCameraManager->GetActorForwardVector()
				, PlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType); //ToDo: change when this is called to improve performance when a gizmo is there without doing anything
}

void ATransformerActor::UpdateTransform(const FVector& LookingVector, const FVector& RayOrigin, const FVector& RayDirection)
{
	if (!Gizmo.IsValid() || CurrentDomain == ETransformationDomain::TD_None) return;

	FVector rayEnd = RayOrigin + 1'000'000'00 * RayDirection;

	FTransform calculatedDeltaTransform = Gizmo->GetDeltaTransform(LookingVector, RayOrigin, rayEnd, CurrentDomain);

	//The delta transform we are actually going to apply (same if there is no Snapping taking place)
	FTransform actualDeltaTransform = calculatedDeltaTransform;


	/* SNAPPING LOGIC */
	bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
	float* snappingValue = SnappingValues.Find(CurrentTransformation);

	if (snappingEnabled && *snappingEnabled && snappingValue)
			actualDeltaTransform = Gizmo->GetSnappedTransform(AccumulatedDeltaTransform
				, calculatedDeltaTransform, CurrentDomain, *snappingValue);
				//GetSnapped Transform Modifies Accumulated Delta Transform by how much Snapping Occurred
		
	for (auto& sc : SelectedComponents)
	{
		if (!sc) continue;
		if (bForceMobility || sc->Mobility == EComponentMobility::Type::Movable)
		{
			const FTransform& componentTransform = sc->GetComponentTransform();

			FQuat deltaRotation = actualDeltaTransform.GetRotation();

			FVector deltaLocation = componentTransform.GetLocation() - Gizmo->GetActorLocation();

			//DeltaScale is Unrotated Scale to Get Local Scale since World Scale is not supported
			FVector deltaScale = componentTransform.GetRotation().UnrotateVector(actualDeltaTransform.GetScale3D());
			
			if(false == bRotateOnLocalAxis)
				deltaLocation = deltaRotation.RotateVector(deltaLocation);

			FTransform newTransform(
				deltaRotation * componentTransform.GetRotation(),
				//adding Gizmo Location + prevDeltaLocation (i.e. location from Gizmo to Object after optional Rotating) + deltaTransform Location Offset
				deltaLocation + Gizmo->GetActorLocation()  + actualDeltaTransform.GetLocation(),
				deltaScale + componentTransform.GetScale3D());			


			/* SNAPPING LOGIC PER COMPONENT */
			if (snappingEnabled && *snappingEnabled && snappingValue)
				newTransform = Gizmo->GetSnappedTransformPerComponent(componentTransform
					, newTransform, CurrentDomain, *snappingValue);

			if (sc->Mobility != EComponentMobility::Type::Movable)
				sc->SetMobility(EComponentMobility::Type::Movable);


			if (GetUFocusableObjects(sc).Num() > 0)
			{
				if (bTransformUFocusableObjects)
					sc->SetWorldTransform(newTransform);
				CallOnNewDeltaTransformation_Internal(sc, newTransform);
			}
			else
				sc->SetWorldTransform(newTransform); //regardless if its local space, DeltaTransform is already adapted to the Space.
		}
		else
			UE_LOG(LogRuntimeTransformer, Warning, TEXT("Transform will not affect Component [%s] as it is NOT Moveable!"), *sc->GetName());

	}
}

bool ATransformerActor::HandleTracedObjects(const TArray<FHitResult>& HitResults
	, bool bClearPreviousSelections, bool bTraceComponent)
{
	//Assign as None just in case we don't hit Any Gizmos
	ClearDomain();

	//Search for our Gizmo (if Valid) First before Selecting any item
	if (Gizmo.IsValid())
	{
		for (auto& hitResult : HitResults)
		{
			if (Gizmo == hitResult.GetActor())
			{
				//Check which Domain of Gizmo was Hit from the Test
				if (USceneComponent* componentHit = Cast<USceneComponent>(hitResult.Component))
				{
					CurrentDomain = Gizmo->GetTransformationDomain(componentHit);
					if (CurrentDomain != ETransformationDomain::TD_None)
					{
						Gizmo->SetTransformProgressState(true, CurrentDomain);
						return true; //finish only if the component actually has a domain, else continue
					}
				}
			}
		}
	}

	//Only consider First Hit
	if (HitResults.Num() > 0)
	{
		if (bTraceComponent)
			SelectComponent(Cast<USceneComponent>(HitResults[0].GetComponent()), bClearPreviousSelections);
		else
			SelectActor(HitResults[0].GetActor(), bClearPreviousSelections);

		return true;
	}
	return false;
}

void ATransformerActor::SetTransformationType(TEnumAsByte<ETransformationType> TransformationType)
{
	//Don't continue if these are the same.
	if (CurrentTransformation == TransformationType) return;

	CurrentTransformation = TransformationType;

	//Clear the Accumulated tranform when we have a new Transformation
	ResetAccumulatedTransform();

	UpdateGizmoPlacement();
}

void ATransformerActor::SetSnappingEnabled(TEnumAsByte<ETransformationType> TransformationType, bool bSnappingEnabled)
{
	SnappingEnabled.Add(TransformationType, bSnappingEnabled);
}

void ATransformerActor::SetSnappingValue(TEnumAsByte<ETransformationType> TransformationType, float SnappingValue)
{
	SnappingValues.Add(TransformationType, SnappingValue);
}

void ATransformerActor::GetSelectedComponents(TArray<class USceneComponent*>& outComponentList, USceneComponent*& outGizmoPlacedComponent) const
{
	outComponentList = SelectedComponents;
	if (Gizmo.IsValid())
		outGizmoPlacedComponent = Gizmo->GetParentComponent();
}

void ATransformerActor::CloneSelected(bool bSelectNewClones, bool bClearPreviousSelections, bool bCloneComponents)
{
	UWorld* world = GetWorld();
	if (!world) return;

	TArray<USceneComponent*> newClones;

	TMap<USceneComponent*, USceneComponent*> cloneParentMap; //cloned component to original parents
	TMap<USceneComponent*, USceneComponent*> originalClonedMap; //cloned components map (original, clone)

	TSet<AActor*> clonedActors; //original actors stored
	
	for (auto& c : SelectedComponents)
	{
		if (!c) continue;
		AActor* owner = c->GetOwner();
		if (!owner) continue;

		USceneComponent* clone = nullptr;
		if (bCloneComponents && c != owner->GetRootComponent())
		{
			//for components, we have to manually create it using duplicateobject
			clone = Cast<USceneComponent>(StaticDuplicateObject(c, owner));
			if (clone)
			{
				//manually call these events
				PostCreateBlueprintComponent(clone);
				clone->OnComponentCreated();
				originalClonedMap.Add(c, clone);
				cloneParentMap.Add(clone, c->GetAttachParent());
				clone->SetRelativeTransform(c->GetRelativeTransform());
			}
		} 

		else
		{
			bool bAlreadyClonedActor;
			clonedActors.Add(owner, &bAlreadyClonedActor);
			if (bAlreadyClonedActor) continue; // if already cloned, don't continue!

			FTransform spawnTransform; //leave it as is since template will also handle the transform
			FActorSpawnParameters spawnParams;
			spawnParams.Template = owner;
			if (AActor* cloneActor = world->SpawnActor(owner->GetClass(), &spawnTransform, spawnParams))
			{
				clone = cloneActor->GetRootComponent();
				if (bCloneComponents)
				{
					originalClonedMap.Add(c, clone);
					//cloneParentMap.Add(clone, c->GetAttachParent());

					TArray<USceneComponent*> children;
					clone->GetChildrenComponents(true, children);
					for (auto& child : children)
						child->DestroyComponent();
				}
			}
		}

		if (clone) 
			newClones.Add(clone);
	}

	//attachment time & registry time for Components
	for (auto& cc : cloneParentMap)
	{
		USceneComponent* parentToAttach = cc.Value;//originalparent

		if (USceneComponent** cloneParent = originalClonedMap.Find(parentToAttach))
		{
			parentToAttach = *cloneParent; //cloned parent
			if (AActor* parentOwner = (*cloneParent)->GetOwner())
				cc.Key->Rename(0, parentOwner); //add to the cloned parent to completely remove relevancy with original parent

			//remove the child from selected if we have a parent that is cloned 
			// having a child selected with its parent will double the transformation of that child
			newClones.Remove(cc.Key); 
		}

		cc.Key->AttachTo(parentToAttach, NAME_None, EAttachLocation::KeepRelativeOffset);
		cc.Key->RegisterComponent();
	}

	if (newClones.Num() > 0 && bSelectNewClones)
		SelectMultipleComponents(newClones, bClearPreviousSelections);
}

void ATransformerActor::SelectComponent(class USceneComponent* Component, bool bClearPreviousSelections)
{
	if (!Component) return;

	if (ShouldSelect(Component->GetOwner(), Component))
	{
		if (bClearPreviousSelections)
			DeselectAll();
		SelectComponent_Internal(Component);
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectActor(AActor* Actor, bool bClearPreviousSelections)
{
	if (!Actor) return;

	if (ShouldSelect(Actor, Actor->GetRootComponent()))
	{
		if (bClearPreviousSelections)
			DeselectAll();
		SelectComponent_Internal(Actor->GetRootComponent());
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectMultipleComponents(const TArray<USceneComponent*>& Components, bool bClearPreviousSelections)
{
	bool bValidList = false;
	for (auto& c : Components)
	{
		if (!c) continue;
		if (!ShouldSelect(c->GetOwner(), c)) continue;

		if (bClearPreviousSelections)
		{
			DeselectAll();
			bClearPreviousSelections = false;
			//only run once. This is not place outside in case a list is empty or contains only invalid components
		}
		bValidList = true;
		SelectComponent_Internal(c);
	}

	if(bValidList) UpdateGizmoPlacement();
}

void ATransformerActor::SelectMultipleActors(const TArray<AActor*>& Actors, bool bClearPreviousSelections)
{
	bool bValidList = false;
	for (auto& a : Actors)
	{
		if (!a) continue;
		if (!ShouldSelect(a, a->GetRootComponent())) continue;

		if (bClearPreviousSelections)
		{
			DeselectAll();
			bClearPreviousSelections = false;
			//only run once. This is not place outside in case a list is empty or contains only invalid components
		}

		bValidList = true;
		SelectComponent_Internal(a->GetRootComponent());
	}

	if(bValidList) UpdateGizmoPlacement();
}

void ATransformerActor::DeselectComponent(USceneComponent* Component)
{
	if (!Component) return;
	DeselectComponent_Internal(Component);
	UpdateGizmoPlacement();
}

void ATransformerActor::DeselectActor(AActor* Actor)
{
	if (Actor)
		DeselectComponent(Actor->GetRootComponent());
}

TArray<USceneComponent*> ATransformerActor::DeselectAll(bool bDestroyDeselected)
{
	auto components = SelectedComponents;
	for (auto& c : components)
		DeselectComponent_Internal(c);
	UpdateGizmoPlacement();

	if (bDestroyDeselected)
	{
		for (auto& c : components)
			c->DestroyComponent();
		components.Empty();
	}

	return components;
}

void ATransformerActor::SelectComponent_Internal(USceneComponent* Component)
{
	if (!Component) return;
	int32 Index = SelectedComponents.Find(Component);

	if (INDEX_NONE == Index) //Component is not in list
	{
		CallFocus_Internal(Component);
		SelectedComponents.Add(Component);
	}
	else if (bToggleSelectedInMultiSelection)
		DeselectComponentByIndex_Internal(Component, Index);
}

void ATransformerActor::DeselectComponent_Internal(USceneComponent* Component)
{
	int32 Index = SelectedComponents.Find(Component);
	if (Index != INDEX_NONE)
		DeselectComponentByIndex_Internal(Component, Index);
}

void ATransformerActor::DeselectComponentByIndex_Internal(USceneComponent* Component, int32 Index)
{
	CallUnfocus_Internal(Component);
	SelectedComponents.RemoveAt(Index);
}

void ATransformerActor::CallFocus_Internal(USceneComponent* Component)
{
	auto focusableObjects = GetUFocusableObjects(Component);

	if (focusableObjects.Num() > 0)
	{
		for (auto& obj : focusableObjects)
			IFocusableObject::Execute_Focus(obj);
	}
	else
		OnComponentSelectionChange(Component, true);


}

void ATransformerActor::CallUnfocus_Internal(USceneComponent* Component)
{
	auto focusableObjects = GetUFocusableObjects(Component);
	if (focusableObjects.Num() > 0)
	{
		for (auto& obj : focusableObjects)
			IFocusableObject::Execute_Unfocus(obj);
	}
	else
		OnComponentSelectionChange(Component, false);
}

void ATransformerActor::CallOnNewDeltaTransformation_Internal(USceneComponent* Component, const FTransform& DeltaTransform)
{
	auto focusableObjects = GetUFocusableObjects(Component);
	for (auto& obj : focusableObjects)
		IFocusableObject::Execute_OnNewDeltaTransformation(obj, DeltaTransform);
}

TArray<UObject*> ATransformerActor::GetUFocusableObjects(USceneComponent* Component) const
{
	TArray<UObject*> focusableObjects;
	if (!Component) return focusableObjects;

	//Add to List if the Component itself Implements our Interface. 
	if (Component->Implements<UFocusableObject>())
		focusableObjects.Add(Component);

	//Add Owner Actor to List if it also Implements our Interface. 
	if (AActor* ComponentOwner = Component->GetOwner())
	{
		if (ComponentOwner->Implements<UFocusableObject>())
			focusableObjects.Add(ComponentOwner);
	}
	return focusableObjects;
}

void ATransformerActor::SetGizmo()
{
	//If there are selected components, then we see whether we need to create a new gizmo.
	if (SelectedComponents.Num() > 0)
	{
		bool bCreateGizmo = true;
		if (Gizmo.IsValid())
		{
			if (CurrentTransformation == Gizmo->GetGizmoType())
			{
				bCreateGizmo = false; // do not create gizmo if there is already a matching gizmo
			}
			else
			{
				// Destroy the current gizmo as the transformation types do not match
				Gizmo->Destroy();
				Gizmo.Reset();
			}
		}

		if (bCreateGizmo)
		{
			if (UWorld* world = GetWorld())
			{
				UE_LOG(LogRuntimeTransformer, Display, TEXT("Creating new Gizmo"));
				UClass* GizmoClass = GetGizmoClass(CurrentTransformation);
				if (GizmoClass)
				{
					Gizmo = Cast<ABaseGizmo>(world->SpawnActor(GizmoClass));
					Gizmo->OnGizmoStateChange.AddDynamic(this, &ATransformerActor::OnGizmoStateChanged);
				}
			}

		}
	}
	//Since there are no selected components, we must destroy any gizmos present
	else
	{
		if (Gizmo.IsValid())
		{
			Gizmo->Destroy();
			Gizmo.Reset();
		}
	}


}

void ATransformerActor::UpdateGizmoPlacement()
{
	SetGizmo();
	//means that there are no active gizmos (no selections) so nothing to do in this func
	if (!Gizmo.IsValid()) return;

	FDetachmentTransformRules detachmentRules(EDetachmentRule::KeepWorld, false);

	// detach from any actors it may be currently attached to
	Gizmo->DetachFromActor(detachmentRules);
	Gizmo->SetActorTransform(FTransform()); //Reset Transformation

	FAttachmentTransformRules attachmentRules = FAttachmentTransformRules::SnapToTargetIncludingScale;

	attachmentRules;

	switch (GizmoPlacement)
	{
	case EGizmoPlacement::GP_OnFirstSelection:
		Gizmo->AttachToComponent(SelectedComponents[0], attachmentRules);
		break;
	case EGizmoPlacement::GP_OnLastSelection:
		Gizmo->AttachToComponent(SelectedComponents.Last(), attachmentRules);
		break;
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType);
}



