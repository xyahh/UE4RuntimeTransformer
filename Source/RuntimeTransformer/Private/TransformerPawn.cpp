// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "TransformerPawn.h"
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
ATransformerPawn::ATransformerPawn()
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

void ATransformerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	ResetDeltaTransform(LastUpdatedDeltaTransform);
	ResetDeltaTransform(AccumulatedDeltaTransform);

	SetTransformationType(CurrentTransformation);
	SetSpaceType(CurrentSpaceType);
}

void ATransformerPawn::SetSpaceType(ESpaceType Type)
{
	CurrentSpaceType = Type;
	SetGizmo();
}

ETransformationDomain ATransformerPawn::GetCurrentDomain(bool& TransformInProgress) const
{
	TransformInProgress = (CurrentDomain != ETransformationDomain::TD_None);
	return CurrentDomain;
}

void ATransformerPawn::ClearDomain()
{
	CurrentDomain = ETransformationDomain::TD_None;

	//Clear the Accumulated tranform when we stop Transforming
	ResetDeltaTransform(AccumulatedDeltaTransform);

	if (Gizmo.IsValid()) 
		Gizmo->SetTransformProgressState(false, CurrentDomain);
}

UClass* ATransformerPawn::GetGizmoClass(ETransformationType TransformationType) const /* private */
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

void ATransformerPawn::ResetDeltaTransform(FTransform& Transform)
{
	Transform = FTransform();
	Transform.SetScale3D(FVector::ZeroVector);
}

bool ATransformerPawn::MouseTraceByObjectTypes(float TraceDistance
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

bool ATransformerPawn::MouseTraceByChannel(float TraceDistance
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

bool ATransformerPawn::MouseTraceByProfile(float TraceDistance
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

bool ATransformerPawn::TraceByObjectTypes(const FVector& StartLocation
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
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerPawn::TraceByChannel(const FVector& StartLocation
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
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

bool ATransformerPawn::TraceByProfile(const FVector& StartLocation
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
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendObjects);
		}
	}
	return false;
}

#include "Kismet/GameplayStatics.h"
void ATransformerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ResetDeltaTransform(LastUpdatedDeltaTransform);
	if (!Gizmo.IsValid()) return;

	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->IsLocalController() && PlayerController->PlayerCameraManager)
		{
			if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
				LastUpdatedDeltaTransform = UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector()
					, worldLocation, worldDirection);
		}			
	}
	
	//Only consider Local View
	if (APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (LocalPlayerController->PlayerCameraManager)
		{
			Gizmo->ScaleGizmoScene(LocalPlayerController->PlayerCameraManager->GetCameraLocation()
				, LocalPlayerController->PlayerCameraManager->GetActorForwardVector()
				, LocalPlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType); //ToDo: change when this is called to improve performance when a gizmo is there without doing anything
}

FTransform ATransformerPawn::UpdateTransform(const FVector& LookingVector
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

void ATransformerPawn::ApplyDeltaTransform(const FTransform& DeltaTransform)
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

bool ATransformerPawn::HandleTracedObjects(const TArray<FHitResult>& HitResults
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

void ATransformerPawn::SetComponentBased(bool bIsComponentBased)
{
	auto selectedComponents = DeselectAll();
	bComponentBased = bIsComponentBased;
	if(bComponentBased)
		SelectMultipleComponents(selectedComponents, false);
	else
		for (auto& c : selectedComponents)
			SelectActor(c->GetOwner());
}

void ATransformerPawn::SetTransformationType(ETransformationType TransformationType)
{
	//Don't continue if these are the same.
	if (CurrentTransformation == TransformationType) return;

	CurrentTransformation = TransformationType;

	//Clear the Accumulated tranform when we have a new Transformation
	ResetDeltaTransform(AccumulatedDeltaTransform);

	UpdateGizmoPlacement();
}

void ATransformerPawn::SetSnappingEnabled(ETransformationType TransformationType, bool bSnappingEnabled)
{
	SnappingEnabled.Add(TransformationType, bSnappingEnabled);
}

void ATransformerPawn::SetSnappingValue(ETransformationType TransformationType, float SnappingValue)
{
	SnappingValues.Add(TransformationType, SnappingValue);
}

void ATransformerPawn::GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
	, USceneComponent*& outGizmoPlacedComponent) const
{
	for (auto& i : SelectedComponents)
		outComponentList.Add(i.Component);
	if (Gizmo.IsValid())
		outGizmoPlacedComponent = Gizmo->GetParentComponent();
}

TArray<FSelectableComponent> ATransformerPawn::GetSelectedComponents() const
{
	return SelectedComponents;
}

void ATransformerPawn::CloneSelected(bool bSelectNewClones
	, bool bAppendObjects)
{
	if (Role != ROLE_Authority)
		UE_LOG(LogRuntimeTransformer, Warning
			, TEXT("Cloning in a Non-Authority! Please use the ReplicatedTransformerPawn Clone RPCs instead"));

	auto ComponentListCopy = SelectedComponents;
	if (false == bAppendObjects)
		DeselectAll();
	CloneFromList(ComponentListCopy, bSelectNewClones);
}

void ATransformerPawn::CloneFromList(const TArray<FSelectableComponent>& ComponentList
	, bool bSelectNewClones)
{
	if (bComponentBased)
	{
		TArray<USceneComponent*> Components;
		for (auto& i : ComponentList)
		{
			if (i.Component)
				Components.Add(i.Component);
		}
		CloneComponents(Components, bSelectNewClones);
	}
	else
	{
		TArray<AActor*> Actors;
		for (auto& i : ComponentList)
		{
			if (i.Component)
				Actors.Add(i.Component->GetOwner());
		}
		CloneActors(Actors, bSelectNewClones);
	}

	if (CurrentDomain != ETransformationDomain::TD_None && Gizmo.IsValid())
		Gizmo->SetTransformProgressState(true, CurrentDomain);
}

void ATransformerPawn::CloneActors(const TArray<AActor*>& Actors
	, bool bSelectNewClones)
{
	UWorld* world = GetWorld();
	if (!world) return;
	
	TSet<AActor*>	actorsProcessed;
	for (auto& templateActor : Actors)
	{
		if (!templateActor) continue;
		bool bAlreadyProcessed;
		actorsProcessed.Add(templateActor, &bAlreadyProcessed);
		if (bAlreadyProcessed) continue;

		FTransform spawnTransform;
		FActorSpawnParameters spawnParams;
		spawnParams.Template = templateActor;

		if (AActor* actor = world->SpawnActor(templateActor->GetClass()
			, &spawnTransform, spawnParams))
		{
			if (bSelectNewClones) 
				SelectActor(actor, false);
		}
	}

}

void ATransformerPawn::CloneComponents(const TArray<class USceneComponent*>& Components
	, bool bSelectNewClones)
{
	UWorld* world = GetWorld();
	if (!world) return;

	TMap<USceneComponent*, USceneComponent*> OcCc; //Original component - Clone component
	TMap<USceneComponent*, USceneComponent*> CcOp; //Clone component - Original parent

	//clone components phase
	for (auto& sc : Components)
	{
		if (!sc) continue;
		AActor* owner = sc->GetOwner();
		if (!owner) continue;
		if (USceneComponent* clone = Cast<USceneComponent>(StaticDuplicateObject(sc, owner)))
		{
			//manually call these events
			PostCreateBlueprintComponent(clone);
			clone->OnComponentCreated();
			clone->SetRelativeTransform(sc->GetRelativeTransform());

			//Add to these two maps for reparenting in next phase
			OcCc.Add(sc, clone); //Original component - Clone component

			if (sc == owner->GetRootComponent())
				CcOp.Add(clone, owner->GetRootComponent()); //this will cause a loop in the maps, so we must check for this!
			else 
				CcOp.Add(clone, sc->GetAttachParent()); //Clone component - Original parent
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

		if(bSelectNewClones && (parent == cp.Value || parent == actorOwner->GetRootComponent())) //check if the parent of the cloned is original (means it's topmost)
			SelectComponent(cp.Key, false); //only select those that have an "original parent". 
		//Selecting childs and parents can cause weird issues so only select the topmost clones (those that do not have cloned parents!)
	}
}

void ATransformerPawn::SelectComponent(class USceneComponent* Component
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

void ATransformerPawn::SelectActor(AActor* Actor
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

void ATransformerPawn::SelectMultipleComponents(const TArray<USceneComponent*>& Components
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

void ATransformerPawn::SelectMultipleActors(const TArray<AActor*>& Actors
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

void ATransformerPawn::DeselectComponent(USceneComponent* Component)
{
	if (!Component) return;
	DeselectComponent_Internal(Component);
	UpdateGizmoPlacement();
}

void ATransformerPawn::DeselectActor(AActor* Actor)
{
	if (Actor)
		DeselectComponent(Actor->GetRootComponent());
}

TArray<USceneComponent*> ATransformerPawn::DeselectAll(bool bDestroyDeselected)
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

void ATransformerPawn::SelectComponent_Internal(USceneComponent* Component)
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

void ATransformerPawn::DeselectComponent_Internal(USceneComponent* Component)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	int32 Index = SelectedComponents.Find(Component);
	if (INDEX_NONE != Index)
		DeselectComponentAtIndex_Internal(Component, Index);
}

void ATransformerPawn::DeselectComponentAtIndex_Internal(USceneComponent* Component
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

void ATransformerPawn::SetGizmo()
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
				UClass* GizmoClass = GetGizmoClass(CurrentTransformation);
				if (GizmoClass)
				{
					UE_LOG(LogRuntimeTransformer, Log,
						TEXT("Creating new Gizmo from class [%s]"), *GizmoClass->GetName());
					Gizmo = Cast<ABaseGizmo>(world->SpawnActor(GizmoClass));
					Gizmo->OnGizmoStateChange.AddDynamic(this, &ATransformerPawn::OnGizmoStateChanged);
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

void ATransformerPawn::UpdateGizmoPlacement()
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
