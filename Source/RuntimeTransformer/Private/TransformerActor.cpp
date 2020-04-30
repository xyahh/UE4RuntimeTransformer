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

	GizmoPlacement			= EGizmoPlacement::GP_OnLastSelection;
	CurrentTransformation = ETransformationType::TT_Translation;
	CurrentDomain		= ETransformationDomain::TD_None;
	CurrentSpaceType = ESpaceType::ST_World;
	TranslationGizmoClass	= ATranslationGizmo::StaticClass();
	RotationGizmoClass		= ARotationGizmo::StaticClass();
	ScaleGizmoClass			= AScaleGizmo::StaticClass();

	bTransformUFocusableObjects = true;
	bRotateOnLocalAxis = false;
	bForceMobility = false;
	bToggleSelectedInMultiSelection = true;
	bComponentBased = false;

}

void ATransformerActor::BeginPlay()
{
	Super::BeginPlay();
	ResetAccumulatedTransform();
	SetTransformationType(CurrentTransformation);
	SetSpaceType(CurrentSpaceType);
}

void ATransformerActor::SetSpaceType(ESpaceType Type)
{
	CurrentSpaceType = Type;
	SetGizmo();
}

ETransformationDomain ATransformerActor::GetCurrentDomain(bool& TransformInProgress) const
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

UClass* ATransformerActor::GetGizmoClass(ETransformationType TransformationType) const /* private */
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

bool ATransformerActor::MouseTraceByObjectTypes(float TraceDistance
	, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors, bool bAppendObjects)
{
	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByObjectTypes(worldLocation, worldLocation + worldDirection * TraceDistance
				, CollisionChannels, IgnoredActors, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerActor::MouseTraceByChannel(float TraceDistance
	, TEnumAsByte<ECollisionChannel> TraceChannel, TArray<AActor*> IgnoredActors
	, bool bAppendObjects)
{
	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByChannel(worldLocation, worldLocation + worldDirection * TraceDistance
				, TraceChannel, IgnoredActors, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerActor::MouseTraceByProfile(float TraceDistance
	, const FName& ProfileName
	, TArray<AActor*> IgnoredActors
	, bool bAppendObjects)
{
	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			return TraceByProfile(worldLocation, worldLocation + worldDirection * TraceDistance
				, ProfileName, IgnoredActors, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerActor::TraceByObjectTypes(const FVector& StartLocation
	, const FVector& EndLocation
	, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors
	, bool bAppendObjects)
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
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerActor::TraceByChannel(const FVector& StartLocation
	, const FVector& EndLocation
	, TEnumAsByte<ECollisionChannel> TraceChannel
	, TArray<AActor*> IgnoredActors
	, bool bAppendObjects)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation
			, TraceChannel, CollisionQueryParams))
		{
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerActor::TraceByProfile(const FVector& StartLocation
	, const FVector& EndLocation
	, const FName& ProfileName, TArray<AActor*> IgnoredActors
	, bool bAppendObjects)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByProfile(OutHits, StartLocation, EndLocation
			, ProfileName, CollisionQueryParams))
		{
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

void ATransformerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Gizmo.IsValid()) return;

	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->PlayerCameraManager)
		{
			if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
				UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector()
					, worldLocation, worldDirection);

			Gizmo->ScaleGizmoScene(PlayerController->PlayerCameraManager->GetCameraLocation()
				, PlayerController->PlayerCameraManager->GetActorForwardVector()
				, PlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType); //ToDo: change when this is called to improve performance when a gizmo is there without doing anything
}

FTransform ATransformerActor::UpdateTransform(const FVector& LookingVector
	, const FVector& RayOrigin
	, const FVector& RayDirection)
{
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector);

	if (!Gizmo.IsValid() || CurrentDomain == ETransformationDomain::TD_None) 
		return deltaTransform;

	FVector rayEnd = RayOrigin + 1'000'000'00 * RayDirection;

	FTransform calcDeltaTransform = Gizmo->GetDeltaTransform(LookingVector, RayOrigin, rayEnd, CurrentDomain);

	//The delta transform we are actually going to apply (same if there is no Snapping taking place)
	deltaTransform = calcDeltaTransform;

	/* SNAPPING LOGIC */
	bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
	float* snappingValue = SnappingValues.Find(CurrentTransformation);

	if (snappingEnabled && *snappingEnabled && snappingValue)
			deltaTransform = Gizmo->GetSnappedTransform(AccumulatedDeltaTransform
				, calcDeltaTransform, CurrentDomain, *snappingValue);
				//GetSnapped Transform Modifies Accumulated Delta Transform by how much Snapping Occurred
	
	ApplyDeltaTransform(deltaTransform);
	return deltaTransform;
}

void ATransformerActor::ApplyDeltaTransform(const FTransform& DeltaTransform)
{
	bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
	float* snappingValue = SnappingValues.Find(CurrentTransformation);

	for (auto& sc : SelectedComponents)
	{
		if (!sc.Component) continue;
		if (bForceMobility || sc.Component->Mobility == EComponentMobility::Type::Movable)
		{
			const FTransform& componentTransform = sc.Component->GetComponentTransform();

			FQuat deltaRotation = DeltaTransform.GetRotation();

			FVector deltaLocation = componentTransform.GetLocation() 
				- Gizmo->GetActorLocation();

			//DeltaScale is Unrotated Scale to Get Local Scale since World Scale is not supported
			FVector deltaScale = componentTransform.GetRotation()
				.UnrotateVector(DeltaTransform.GetScale3D());


			if (false == bRotateOnLocalAxis)
				deltaLocation = deltaRotation.RotateVector(deltaLocation);

			FTransform newTransform(
				deltaRotation * componentTransform.GetRotation(),
				//adding Gizmo Location + prevDeltaLocation 
				// (i.e. location from Gizmo to Object after optional Rotating)
				// + deltaTransform Location Offset
				deltaLocation + Gizmo->GetActorLocation() + DeltaTransform.GetLocation(),
				deltaScale + componentTransform.GetScale3D());


			/* SNAPPING LOGIC PER COMPONENT */
			if (snappingEnabled && *snappingEnabled && snappingValue)
				newTransform = Gizmo->GetSnappedTransformPerComponent(componentTransform
					, newTransform, CurrentDomain, *snappingValue);

			if (sc.Component->Mobility != EComponentMobility::Type::Movable)
				sc.Component->SetMobility(EComponentMobility::Type::Movable);

			sc.SetTransform(newTransform, bTransformUFocusableObjects, bComponentBased);

		}
		else
			UE_LOG(LogRuntimeTransformer, Warning
				, TEXT("Transform will not affect Component [%s] as it is NOT Moveable!")
				, *sc.Component->GetName());
	}
}

bool ATransformerActor::HandleTracedObjects(const TArray<FHitResult>& HitResults
	, bool bAppendObjects)
{
	//Assign as None just in case we don't hit Any Gizmos
	ClearDomain();

	//Search for our Gizmo (if Valid) First before Selecting any item
	if (Gizmo.IsValid())
	{
		for (auto& hitResult : HitResults)
		{
			if (Gizmo == hitResult.GetActor()) //check if it's OUR gizmo
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

	for (auto& hits : HitResults)
	{
		if (Cast<ABaseGizmo>(hits.Actor))
			continue; //ignore other Gizmos.

		if (bComponentBased)
			SelectComponent(Cast<USceneComponent>(hits.GetComponent()), bAppendObjects);
		else
			SelectActor(hits.GetActor(), bAppendObjects);

		return true;
	}
	return false;
}

void ATransformerActor::SetComponentBased(bool bIsComponentBased)
{
	auto selectedComponents = DeselectAll();
	bComponentBased = bIsComponentBased;
	if(bComponentBased)
		SelectMultipleComponents(selectedComponents, false);
	else
		for (auto& c : selectedComponents)
			SelectActor(c->GetOwner());
}

void ATransformerActor::SetTransformationType(ETransformationType TransformationType)
{
	//Don't continue if these are the same.
	if (CurrentTransformation == TransformationType) return;

	CurrentTransformation = TransformationType;

	//Clear the Accumulated tranform when we have a new Transformation
	ResetAccumulatedTransform();

	UpdateGizmoPlacement();
}

void ATransformerActor::SetSnappingEnabled(ETransformationType TransformationType, bool bSnappingEnabled)
{
	SnappingEnabled.Add(TransformationType, bSnappingEnabled);
}

void ATransformerActor::SetSnappingValue(ETransformationType TransformationType, float SnappingValue)
{
	SnappingValues.Add(TransformationType, SnappingValue);
}

void ATransformerActor::GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
	, USceneComponent*& outGizmoPlacedComponent) const
{
	for (auto& i : SelectedComponents)
		outComponentList.Add(i.Component);
	if (Gizmo.IsValid())
		outGizmoPlacedComponent = Gizmo->GetParentComponent();
}

void ATransformerActor::CloneSelected(const UObject* WorldContext, bool bSelectNewClones
	, bool bAppendObjects)
{
	if (bComponentBased)
		CloneSelected_Components(WorldContext, bSelectNewClones, bAppendObjects);
	else
		CloneSelected_Actors(WorldContext, bSelectNewClones, bAppendObjects);

	if (CurrentDomain != ETransformationDomain::TD_None && Gizmo.IsValid())
		Gizmo->SetTransformProgressState(true, CurrentDomain);
}

void ATransformerActor::CloneSelected_Actors(const UObject* WorldContext
	, bool bSelectNewClones
	, bool bAppendObjects)
{
	UWorld* world = GetWorld();
	if (!world) return;
	
	if (Role != ROLE_Authority)
		UE_LOG(LogRuntimeTransformer, Warning, TEXT("Clone is not being executed in the Server!"));

	TArray<AActor*> actorsToClone; //an array to keep order
	for (auto& sc : SelectedComponents)
	{
		if(sc.Component)
			actorsToClone.Add(sc.Component->GetOwner());
	}

	TArray<AActor*> actorClones;
	TSet<AActor*>	actorsProcessed;
	
	if (false == bAppendObjects)
		DeselectAll(false);

	for (auto& templateActor : actorsToClone)
	{
		if (!templateActor) continue;
		bool bAlreadyProcessed;
		actorsProcessed.Add(templateActor, &bAlreadyProcessed);
		if (bAlreadyProcessed) continue;

		FTransform spawnTransform;
		FActorSpawnParameters spawnParams;
		spawnParams.Template = templateActor;
		if (AActor* actor = world->SpawnActor(templateActor->GetClass(), &spawnTransform, spawnParams))
		{
			if (bSelectNewClones) SelectActor(actor, false);
		}
	}

}

void ATransformerActor::CloneSelected_Components(const UObject* WorldContext
	, bool bSelectNewClones
	, bool bAppendObjects)
{
	UWorld* world = GetWorld();
	if (!world) return;

	TMap<USceneComponent*, USceneComponent*> OcCc; //Original component - Clone component
	TMap<USceneComponent*, USceneComponent*> CcOp; //Clone component - Original parent

	auto Components = SelectedComponents;

	if (false == bAppendObjects)
		DeselectAll();

	//clone components phase
	for (auto& sc : Components)
	{
		if (!sc.Component) continue;
		AActor* owner = sc.Component->GetOwner();
		if (!owner) continue;
		if (USceneComponent* clone = Cast<USceneComponent>(StaticDuplicateObject(sc.Component, owner)))
		{
			//manually call these events
			PostCreateBlueprintComponent(clone);
			clone->OnComponentCreated();
			clone->SetRelativeTransform(sc.Component->GetRelativeTransform());

			//Add to these two maps for reparenting in next phase
			OcCc.Add(sc.Component, clone); //Original component - Clone component

			if (sc.Component == owner->GetRootComponent())
				CcOp.Add(clone, owner->GetRootComponent()); //this will cause a loop in the maps, so we must check for this!
			else 
				CcOp.Add(clone, sc.Component->GetAttachParent()); //Clone component - Original parent
		}
	}

	//reparenting phase
	FAttachmentTransformRules attachmentRule(EAttachmentRule::KeepWorld, false);
	for (auto& cp : CcOp)
	{
		//original parent
		USceneComponent* parent = cp.Value; 

		AActor* actorOwner = cp.Value->GetOwner();

		//find if we cloned the original parent
		USceneComponent** cloneParent = OcCc.Find(parent); 

		if (cloneParent)
		{
			if (*cloneParent != cp.Key) //make sure comp is not its own parent
				parent = *cloneParent;
		}
		else 
		{
			//couldn't find its parent, so find the parent of the parent and see if it's in the list.
			//repeat until found or root is reached
			while (1)
			{
				//if parent is root, then no need to find parents above it. (none)
				//attach to original parent, since there are no cloned parents.
				if (parent == actorOwner->GetRootComponent())
				{
					parent = cp.Value;
					break;
				}
				
				//check if parents have been cloned
				cloneParent = OcCc.Find(parent->GetAttachParent());
				if (cloneParent)
				{
					//attach to cloned parent if found
					parent = *cloneParent;
					break;
				}
				parent = parent->GetAttachParent(); //move up in the hierarchy
			}
		}

		cp.Key->AttachToComponent(parent, attachmentRule);
		cp.Key->RegisterComponent();

		if(parent == cp.Value || parent == actorOwner->GetRootComponent()) //check if the parent of the cloned is original (means it's topmost)
			SelectComponent(cp.Key, false); //only select those that have an "original parent". 
		//Selecting childs and parents can cause weird issues so only select the topmost clones (those that do not have cloned parents!)
	}
}

void ATransformerActor::SelectComponent(class USceneComponent* Component
	, bool bAppendObjects)
{
	if (!Component) return;

	if (ShouldSelect(Component->GetOwner(), Component))
	{
		if (false == bAppendObjects)
			DeselectAll();
		SelectComponent_Internal(Component);
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectActor(AActor* Actor
	, bool bAppendObjects)
{
	if (!Actor) return;

	if (ShouldSelect(Actor, Actor->GetRootComponent()))
	{
		if (false == bAppendObjects)
			DeselectAll();
		SelectComponent_Internal(Actor->GetRootComponent());
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectMultipleComponents(const TArray<USceneComponent*>& Components
	, bool bAppendObjects)
{
	bool bValidList = false;
	for (auto& c : Components)
	{
		if (!c) continue;
		if (!ShouldSelect(c->GetOwner(), c)) continue;

		if (false == bAppendObjects)
		{
			DeselectAll();
			bAppendObjects = true;
			//only run once. This is not place outside in case a list is empty or contains only invalid components
		}
		bValidList = true;
		SelectComponent_Internal(c);
	}

	if(bValidList) UpdateGizmoPlacement();
}

void ATransformerActor::SelectMultipleActors(const TArray<AActor*>& Actors
	, bool bAppendObjects)
{
	bool bValidList = false;
	for (auto& a : Actors)
	{
		if (!a) continue;
		if (!ShouldSelect(a, a->GetRootComponent())) continue;

		if (false == bAppendObjects)
		{
			DeselectAll();
			bAppendObjects = true;
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
	TArray<USceneComponent*> outComponents;

	auto components = SelectedComponents;
	for (auto& c : components)
	{
		outComponents.Add(c.Component);
		DeselectComponent(c.Component);
	}

	SelectedComponents.Empty();
	UpdateGizmoPlacement();

	if (bDestroyDeselected)
	{
		for (auto& c : components)
		{
			if (!IsValid(c.Component)) continue; //a component that was in the same actor destroyed will be pending kill
			if (AActor* actor = c.Component->GetOwner())
			{
				//We destroy the actor if no components are left to destroy, or the system is currently ActorBased
				if (bComponentBased && actor->GetComponents().Num() > 1)
					c.Component->DestroyComponent(true);
				else
					actor->Destroy();
			}
		}
	}

	return outComponents;
}

void ATransformerActor::SelectComponent_Internal(USceneComponent* Component)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	int32 Index = SelectedComponents.Find(Component);

	if (INDEX_NONE == Index) //Component is not in list
	{
		SelectedComponents.Emplace(Component);
		bool bImplementsInterface;
		SelectedComponents.Last().Select(bComponentBased, &bImplementsInterface);
		OnComponentSelectionChange(Component, true, bImplementsInterface);
	}
	else if (bToggleSelectedInMultiSelection)
		DeselectComponentAtIndex_Internal(Component, Index);
}

void ATransformerActor::DeselectComponent_Internal(USceneComponent* Component)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	int32 Index = SelectedComponents.Find(Component);
	if (INDEX_NONE != Index)
		DeselectComponentAtIndex_Internal(Component, Index);
}

void ATransformerActor::DeselectComponentAtIndex_Internal(USceneComponent* Component
	, int32 Index)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	if (SelectedComponents.IsValidIndex(Index))
	{
		bool bImplementsInterface;
		SelectedComponents[Index].Deselect(bComponentBased, &bImplementsInterface);
		SelectedComponents.RemoveAt(Index);
		OnComponentSelectionChange(Component, false, bImplementsInterface);
	}

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

	USceneComponent* ComponentToAttachTo = nullptr;

	switch (GizmoPlacement)
	{
	case EGizmoPlacement::GP_OnFirstSelection: 
		ComponentToAttachTo = SelectedComponents[0].Component; break;
	case EGizmoPlacement::GP_OnLastSelection:
		ComponentToAttachTo = SelectedComponents.Last().Component; break;
	}

	if (ComponentToAttachTo)
	{
		FAttachmentTransformRules attachmentRules 
			= FAttachmentTransformRules::SnapToTargetIncludingScale;
		Gizmo->AttachToComponent(ComponentToAttachTo, attachmentRules);
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType);
}
