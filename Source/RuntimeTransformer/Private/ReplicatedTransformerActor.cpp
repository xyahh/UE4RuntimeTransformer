// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicatedTransformerActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AReplicatedTransformerActor::AReplicatedTransformerActor()
{
	bIgnoreNonReplicatedObjects = true;

	NetworkDeltaTransform = FTransform();
	NetworkDeltaTransform.SetScale3D(FVector::ZeroVector);
}

void AReplicatedTransformerActor::Tick(float DeltaSeconds)
{
	Super::Super::Tick(DeltaSeconds);//we don't Tick Transformer Actor because we need some extra functionality

	if (APlayerController* PlayerController = Cast< APlayerController>(Controller))
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->PlayerCameraManager)
		{
			if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
			{
				FTransform deltaTransform = UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector()
					, worldLocation, worldDirection);

				NetworkDeltaTransform = FTransform(
					deltaTransform.GetRotation() * NetworkDeltaTransform.GetRotation(),
					deltaTransform.GetLocation() + NetworkDeltaTransform.GetLocation(),
					deltaTransform.GetScale3D() + NetworkDeltaTransform.GetScale3D()
				);
			}
			//Gizmo->ScaleGizmoScene(PlayerController->PlayerCameraManager->GetCameraLocation()
			//	, PlayerController->PlayerCameraManager->GetActorForwardVector()
			//	, PlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}
}

void AReplicatedTransformerActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AReplicatedTransformerActor, bIgnoreNonReplicatedObjects);
}



void AReplicatedTransformerActor::ServerTraceByObjectTypes_Implementation(const FVector& StartLocation, const FVector& EndLocation, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels, bool bAppendObjects)
{
	MulticastTraceByObjectTypes(StartLocation,
		EndLocation, CollisionChannels, bAppendObjects);
}

void AReplicatedTransformerActor::MulticastTraceByObjectTypes_Implementation(const FVector& StartLocation, const FVector& EndLocation, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels, bool bAppendObjects)
{
	bool bTracedObject = TraceByObjectTypes(StartLocation, EndLocation, CollisionChannels
		, TArray<AActor*>(), bAppendObjects);

	if (bIgnoreNonReplicatedObjects)
	{
		TArray<USceneComponent*> selectedComponents;
		TArray<USceneComponent*> componentsToDeselect;
		USceneComponent* gizmoPlacedComponent;
		GetSelectedComponents(selectedComponents, gizmoPlacedComponent);

		bool bComponent = GetComponentBased();
		for (auto& i : selectedComponents)
		{
			AActor* owner = i->GetOwner();
			if (owner && owner->GetIsReplicated())
			{
				if (bComponent)
				{
					if (i->GetIsReplicated())
						continue; //components - actor owner + themselves need to replicate
				} else
					continue; //actors only consider whether they replicate
			}			
			DeselectComponent(i);
		}
	}
	

	if (!bTracedObject && !bAppendObjects)
		DeselectAll();
}



void AReplicatedTransformerActor::ServerClearDomain_Implementation()
{
	MulticastClearDomain();
}

void AReplicatedTransformerActor::MulticastClearDomain_Implementation()
{
	ClearDomain();
}



void AReplicatedTransformerActor::ServerApplyTransform_Implementation(const FTransform& DeltaTransform)
{
	MulticastApplyTransform(DeltaTransform);
}

void AReplicatedTransformerActor::MulticastApplyTransform_Implementation(const FTransform& DeltaTransform)
{
	//reflect this change on others but owner
	if (false == IsOwnedBy(UGameplayStatics::GetPlayerController(this, 0)))
		ApplyDeltaTransform(DeltaTransform);
}

void AReplicatedTransformerActor::FinishTransform()
{
	ServerClearDomain();
	ServerApplyTransform(NetworkDeltaTransform);

	NetworkDeltaTransform = FTransform();
	NetworkDeltaTransform.SetScale3D(FVector::ZeroVector);
}



void AReplicatedTransformerActor::ServerDeselectAll_Implementation(bool bDestroySelected)
{
	MulticastDeselectAll(bDestroySelected);
}

void AReplicatedTransformerActor::MulticastDeselectAll_Implementation(bool bDestroySelected)
{
	DeselectAll(bDestroySelected);
}



void AReplicatedTransformerActor::ServerSetSpaceType_Implementation(ESpaceType Space)
{
	MulticastSetSpaceType(Space);
}

void AReplicatedTransformerActor::MulticastSetSpaceType_Implementation(ESpaceType Space)
{
	SetSpaceType(Space);
}
