// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicatedTransformerPawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Gizmos/BaseGizmo.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

AReplicatedTransformerPawn::AReplicatedTransformerPawn()
{
	bIgnoreNonReplicatedObjects = true;

	NetworkDeltaTransform = FTransform();
	NetworkDeltaTransform.SetScale3D(FVector::ZeroVector);
}

void AReplicatedTransformerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	NetworkDeltaTransform = FTransform(
		LastUpdatedDeltaTransform.GetRotation() * NetworkDeltaTransform.GetRotation(),
		LastUpdatedDeltaTransform.GetLocation() + NetworkDeltaTransform.GetLocation(),
		LastUpdatedDeltaTransform.GetScale3D() + NetworkDeltaTransform.GetScale3D());
}

void AReplicatedTransformerPawn::FilterHits(TArray<FHitResult>& outHits)
{
	//eliminate all outHits that have non-replicated objects
	if (bIgnoreNonReplicatedObjects)
	{
		bool bComponent = GetComponentBased();
		TArray<FHitResult> checkHits = outHits;
		for (int32 i = 0; i < checkHits.Num(); ++i)
		{
			//don't remove Gizmos! They do not replicate by default 
			if (Cast<ABaseGizmo>(checkHits[i].Actor))
				continue;

			if (checkHits[i].Actor->GetIsReplicated())
			{
				if (bComponent)
				{
					if (checkHits[i].Component->GetIsReplicated())
						continue; //components - actor owner + themselves need to replicate
				}
				else
					continue; //actors only consider whether they replicate
			}
			UE_LOG(LogRuntimeTransformer, Log
				, TEXT("Removing (Actor: %s   ComponentHit:  %s) from hits because it does not satisfy replication requirements")
				, *checkHits[i].Actor->GetName(), *checkHits[i].Component->GetName());
			outHits.RemoveAt(i);
		}
	}
}

void AReplicatedTransformerPawn::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AReplicatedTransformerPawn, bIgnoreNonReplicatedObjects);
}


void AReplicatedTransformerPawn::ServerTraceByObjectTypes_Implementation(const FVector& StartLocation, const FVector& EndLocation, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels, bool bAppendObjects)
{
	MulticastTraceByObjectTypes(StartLocation,
		EndLocation, CollisionChannels, bAppendObjects);
}

void AReplicatedTransformerPawn::MulticastTraceByObjectTypes_Implementation(const FVector& StartLocation, const FVector& EndLocation, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels, bool bAppendObjects)
{
	bool bTracedObject = TraceByObjectTypes(StartLocation, EndLocation
		, CollisionChannels, TArray<AActor*>(), bAppendObjects);

	if (!bTracedObject && !bAppendObjects)
		DeselectAll();
}


void AReplicatedTransformerPawn::ServerClearDomain_Implementation()
{
	MulticastClearDomain();
}

void AReplicatedTransformerPawn::MulticastClearDomain_Implementation()
{
	ClearDomain();
}



void AReplicatedTransformerPawn::ServerApplyTransform_Implementation(const FTransform& DeltaTransform)
{
	MulticastApplyTransform(DeltaTransform);
}

void AReplicatedTransformerPawn::MulticastApplyTransform_Implementation(const FTransform& DeltaTransform)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if(false == PlayerController->IsLocalController()) //only apply to others
			ApplyDeltaTransform(DeltaTransform);
	}
}

void AReplicatedTransformerPawn::FinishTransform()
{
	ServerClearDomain();
	ServerApplyTransform(NetworkDeltaTransform);

	NetworkDeltaTransform = FTransform();
	NetworkDeltaTransform.SetScale3D(FVector::ZeroVector);
}

void AReplicatedTransformerPawn::ServerDeselectAll_Implementation(bool bDestroySelected)
{
	MulticastDeselectAll(bDestroySelected);
}

void AReplicatedTransformerPawn::MulticastDeselectAll_Implementation(bool bDestroySelected)
{
	DeselectAll(bDestroySelected);
}


void AReplicatedTransformerPawn::ServerSetSpaceType_Implementation(ESpaceType Space)
{
	MulticastSetSpaceType(Space);
}

void AReplicatedTransformerPawn::MulticastSetSpaceType_Implementation(ESpaceType Space)
{
	SetSpaceType(Space);
}

void AReplicatedTransformerPawn::ServerCloneSelected_Implementation(bool bSelectNewClones, bool bAppendObjects)
{
	//CloneSelected(bSelectNewClones, bAppendObjects);

	auto ComponentListCopy = GetSelectedComponents();
	if (false == bAppendObjects)
		MulticastDeselectAll(false);
	CloneFromList(ComponentListCopy, bSelectNewClones);
}
