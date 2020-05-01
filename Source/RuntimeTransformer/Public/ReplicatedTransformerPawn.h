// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TransformerPawn.h"
#include "ReplicatedTransformerPawn.generated.h"

/**
 * 
 */
UCLASS()
class RUNTIMETRANSFORMER_API AReplicatedTransformerPawn : public ATransformerPawn
{
	GENERATED_BODY()
	
public:

	AReplicatedTransformerPawn();

protected:

	virtual void Tick(float DeltaSeconds) override;

	virtual void FilterHits(TArray<FHitResult>& outHits) override;

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& LifetimeProps) const override;

	/*
	* ServerCall
	* @ see TraceByObjectTypes
	*/
	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerTraceByObjectTypes(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bAppendObjects);

	//SERVER TRACE BY OBJECT TYPES
	bool ServerTraceByObjectTypes_Validate(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bAppendObjects) { return true; }

	void ServerTraceByObjectTypes_Implementation(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bAppendObjects);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastTraceByObjectTypes(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bAppendObjects);

	void MulticastTraceByObjectTypes_Implementation(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bAppendObjects);

	/*
	* ServerCall
	* @ see ClearDomain
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerClearDomain();

	//SERVER CLEAR DOMAIN
	bool ServerClearDomain_Validate() { return true; }
	void ServerClearDomain_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastClearDomain();

	void MulticastClearDomain_Implementation();

	/*
	* ServerCall
	* @ see ApplyTransform
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerApplyTransform(const FTransform& DeltaTransform);
	
	//SERVER APPLY TRANSFORM
	bool ServerApplyTransform_Validate(const FTransform& DeltaTransform) { return true; }
	void ServerApplyTransform_Implementation(const FTransform& DeltaTransform);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyTransform(const FTransform& DeltaTransform);

	void MulticastApplyTransform_Implementation(const FTransform& DeltaTransform);

	/*
	* Calls the ServerClearDomain.
	* Then it calls ServerApplyTransform and Resets the Accumulated Network Transform.
	*/
	UFUNCTION(BlueprintCallable, Category = "Replicated Runtime Transformer")
	void FinishTransform();

	/*
	* ServerCall
	* @ see DeselectAll
	*/
	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerDeselectAll(bool bDestroySelected);


public:

	//SERVER DESELECT ALL
	bool ServerDeselectAll_Validate(bool bDestroySelected) { return true; }
	void ServerDeselectAll_Implementation(bool bDestroySelected);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDeselectAll(bool bDestroySelected);

	void MulticastDeselectAll_Implementation(bool bDestroySelected);


	/*
	* ServerCall
	* @ see SetSpaceType
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerSetSpaceType(ESpaceType Space);

	//SERVER SET SPACE TYPE
	bool ServerSetSpaceType_Validate(ESpaceType Space) { return true; }
	void ServerSetSpaceType_Implementation(ESpaceType Space);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetSpaceType(ESpaceType Space);
	void MulticastSetSpaceType_Implementation(ESpaceType Space);



	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = "Replicated Runtime Transformer")
	void ServerCloneSelected(bool bSelectNewClones = true
		, bool bAppendObjects = false);

	bool ServerCloneSelected_Validate(bool bSelectNewClones
		, bool bAppendObjects) { return true; }

	void ServerCloneSelected_Implementation(bool bSelectNewClones
		, bool bAppendObjects);

public:

	/*
	* Ignore Non-Replicated Objects means that the objects that do not satisfy
	* the replication conditions will become unselectable. This only takes effect if using the ServerTracing
	* The Replication Conditions:
	* - For an actor, replicating must be on
	* - For a component, both its owner and itself need to be replicating
	*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Replicated Runtime Transformer")
	bool bIgnoreNonReplicatedObjects;

	FTransform NetworkDeltaTransform;

};
