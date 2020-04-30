// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeTransformer.h"
#include "RuntimeTransformer/Private/SelectableComponent.h"
#include "TransformerActor.generated.h"

UENUM(BlueprintType)
enum class EGizmoPlacement : uint8
{
	GP_None					UMETA(DisplayName = "None"),
	GP_OnFirstSelection		UMETA(DisplayName = "On First Selection"),
	GP_OnLastSelection		UMETA(DisplayName = "On Last Selection"),
};

UCLASS()
class RUNTIMETRANSFORMER_API ATransformerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATransformerActor();
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;

protected:

	virtual void BeginPlay() override;

public:

	/*
	* This gets called everytime a Component / Actor is going to get added.
	* The default return is TRUE, but it can be overriden to check for additional things 
	* (e.g. checking if it implements an interface, has some property, is child of a class, etc)

	* @param OwnerActor: The Actor owning the Component Selected 
	* @param Component: The Component Selected (if it's an Actor Selected, this would be its RootComponent)

	* @return bool: Whether or not this Component should be added.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	bool ShouldSelect(AActor* OwnerActor, class USceneComponent* Component);

	//by default return true, override for custom logic
	virtual bool ShouldSelect_Implementation(AActor* OwnerActor, class USceneComponent* Component) { return true; }

	/**
	 * Setting a Player Controller makes most functionality automatic, but not necessary.
	 * The Player Controller is used to get the Mouse World Space Position & Direction for the given Transformations.

	 * NOTE: If the Player Controller is NOT set,  Tracing & UpdateTransform functions must be called manually.
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetPlayerController(class APlayerController* Controller);

	//Sets the Space of the Gizmo, whether its Local or World space.
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSpaceType(ESpaceType Type);

	/*
	 * Gets the Current Domain
	 * If it returns ETransformationDomain::TD_None, then that means
	 * there is no Transformation in Progress.
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	ETransformationDomain GetCurrentDomain(bool& TransformInProgress) const;


	/**
	 * Sets the Current Domain to NONE. (Transforming in Progress will become false)
	 * Should be called when we are done transforming with the Gizmo.

	 * @param bReplicateChanges - whether we want the transformations that took place to be replicated to the Server
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void ClearDomain();

	/*
	Server call for the ClearDomain Func
	@see ClearDomain
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Runtime Transformer")
	void ServerClearDomain();
	bool ServerClearDomain_Validate();
	void ServerClearDomain_Implementation();


	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	  * NOTE: SHOULD BE CALLED BY THE CLIENT!

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer Client")
	bool MouseTraceByObjectTypes(float TraceDistance
		, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	  * NOTE: SHOULD BE CALLED BY THE CLIENT!

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param TraceChannel - The Ray Collision Channel to be Considered in the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer Client")
	bool MouseTraceByChannel(float TraceDistance, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * NOTE: SHOULD BE CALLED BY THE CLIENT!

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param ProfileName - The Profile Name to be used during the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer Client")
	bool MouseTraceByProfile(float TraceDistance, const FName& ProfileName
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace (Not used if Caller is Not Server)
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByObjectTypes(const FVector& StartLocation, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/*
	Server call for the TraceByObjectTypes Func
	* note that there's no list of Ignored actors. The Ignored actors can only be set my the server
	@see TraceByObjectTypes
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Runtime Transformer Server")
	void ServerTraceByObjectTypes(const FVector& StartLocation, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bClearPreviousSelections = true);

	bool ServerTraceByObjectTypes_Validate(const FVector& StartLocation, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bClearPreviousSelections = true);

	void ServerTraceByObjectTypes_Implementation(const FVector& StartLocation
		, const FVector& EndLocation
		, const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
		, bool bClearPreviousSelections = true);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param TraceChannel - The Ray Collision Channel to be Considered in the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByChannel(const FVector& StartLocation, const FVector& EndLocation
		, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/*
	* Server call for TraceByChannel func
	* @see TraceByChannel
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Runtime Transformer")
	void ServerTraceByChannel(const FVector& StartLocation, const FVector& EndLocation
		, ECollisionChannel TraceChannel
		, bool bClearPreviousSelections = true);

	bool ServerTraceByChannel_Validate(const FVector& StartLocation, const FVector& EndLocation
		, ECollisionChannel TraceChannel
		, bool bClearPreviousSelections = true);

	void ServerTraceByChannel_Implementation(const FVector& StartLocation, const FVector& EndLocation
		, ECollisionChannel TraceChannel
		, bool bClearPreviousSelections = true);


	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param ProfileName - The Profile Name to be used during the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Components hit (true), or Actors hit (false)
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByProfile(const FVector& StartLocation, const FVector& EndLocation
		, const FName& ProfileName
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true);

	/*
	 * Server call for TraceByProfile func
	 * @see TraceByProfile
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Runtime Transformer Server")
	void ServerTraceByProfile(const FVector& StartLocation
		, const FVector& EndLocation
		, const FName& ProfileName
		, bool bClearPreviousSelections = true);

	bool ServerTraceByProfile_Validate(const FVector& StartLocation
		, const FVector& EndLocation
		, const FName& ProfileName
		, bool bClearPreviousSelections = true);

	void ServerTraceByProfile_Implementation(const FVector& StartLocation
		, const FVector& EndLocation
		, const FName& ProfileName
		, bool bClearPreviousSelections = true);


	// Update every Frame
	// Checks for Mouse Update
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * If the Gizmo is currently in a Valid Domain,
	 * it will transform the Selected Object(s) through a valid domain.
	 * The transform is calculated with the given Ray Origin and Ray Direction in World Space (usually the Mouse World Position & Mouse World Direction)

	 * This function should be called if NO Player Controller has been Set

	 * @param LookingVector - The looking direction of the player (e.g. Camera Forward Vector)
	 * @param RayOrigin - The origin point (world space) of the Ray (e.g. the Mouse Position in World Space)
	 * @param RayDirection - the direction of the ray (in world space) (e.g. the Mouse Direction in World Space)
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void UpdateTransform(const FVector& LookingVector, const FVector& RayOrigin
		, const FVector& RayDirection);

	void ApplyDeltaTransform(const FTransform& AccumulatedDeltaTransform);

	/*
	* Server call for "UpdateTransform"
	* @see UpdateTransform
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Runtime Transformer Server")
	void ServerFinishUpdating(const FTransform& AccumulatedDeltaTransform);

	bool ServerFinishUpdating_Validate(const FTransform& AccumulatedDeltaTransform);

	void ServerFinishUpdating_Implementation(const FTransform& AccumulatedDeltaTransform);
	

	/**
	 * Processes the OutHits generated by Tracing and Selects either a Gizmo (priority) or
	 * if no Gizmo is present in the trace, the first object hit is selected.

	 * If not called on Server, only calling Client will see the changes.

	 * This is already called by the RuntimeTransformer built-in Trace Functions,
	 * but can be called manually if you wish to provide your own list of Hit Results (e.g. tracing with different configuration/method)
	 *
	 * @param HitResults - a list of the FHitResults that were generated by LineTracing
	 * @param bClearPrevious Selections - whether we should clear the previously selected objects (only relevant when there is no Gizmo in hit list)
	 * @param bTraceComponent - whether we should get the HitComponent or not. If false, the HitActor will be used instead.
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer Server")
	bool HandleTracedObjects(const TArray<FHitResult>& HitResults
		, bool bClearPreviousSelections = true);

	/*
	 * Called when the Gizmo State has changed (i.e. Domain has changed)
	 * @param GizmoType - the type of Gizmo that was changed (Translation, Rotation or Scale)
	 * @param bTransformInProgress - whether the Transform is currently in progress. This is basically a bool that evaluates to Domain != NONE
	 * @param Domain - The current domain that the Gizmo State changed to
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	void OnGizmoStateChanged(ETransformationType GizmoType
		, bool bTransformInProgress, ETransformationDomain Domain);

	virtual void OnGizmoStateChanged_Implementation(ETransformationType GizmoType
		, bool bTransformInProgress, ETransformationDomain Domain)
	{
		//this should be overriden for custom logic
	}

	/*
	 * Called when a new Component has been Selected (Focused)
	 * or has been unselected (unfocused).

	 * This should be used if there needs to be logic applied to
	 * objects that do not implement the UFocusable interface.

	 * @param Component - the Component selected/deselected
	 * @param bSelected - whether the given component was selected or unselected
	 * @param bImplementsUFocusable - whether the given Component implements UFocusable (Actor considered, if actors are traced)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	void OnComponentSelectionChange(class USceneComponent* Component, bool bSelected
		, bool bImplementsUFocusable);

	virtual void OnComponentSelectionChange_Implementation(class USceneComponent* Component
		, bool bSelected, bool bImplementsUFocusable)
	{
		//This should be overriden for custom logic
	}

public:

	/**
	 * Whether to Set the System to work with Components (true)
	 * or to work with Actors (false)

	 @see bComponentBased
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetComponentBased(bool bIsComponentBased);

	bool GetComponentBased() const { return bComponentBased; }

	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetTransformationType(ETransformationType TransformationType);
	
	/*
	 * Enables/Disables Snapping for a given Transformation
	 * Snapping Value for the Given Transformation MUST NOT be 0 for Snapping to work

	 @see SetSnappingValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSnappingEnabled(ETransformationType TransformationType, bool bSnappingEnabled);

	/*
	 * Sets a Snapping Value for a given Transformation
	 * Snapping Value MUST NOT be 0  and Snapping must be enabled for the given transformation for Snapping to work

	 @see SetSnappingEnabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSnappingValue(ETransformationType TransformationType, float SnappingValue);

	/*
	 * Gets the list of Selected Components.

	 @return outComponentList - the List of Currently Selected Components
	 @return outGizmoPlacedComponent - the Component in the list that currently has the Gizmo attached
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
		, class USceneComponent*& outGizmoPlacedComponent) const;

	/*
	* Makes an exact copy of the Actors that are owners of the components and makes
	* a copy of them.
	
	* Don't spam this :)

	* @param bSelectNewClones - whether to add the new clones to the Selection
	* @param bClearPreviousSelections Whether to clear the previous selected Components 
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void CloneSelected(bool bSelectNewClones = true , bool bClearPreviousSelections = true);

private:

	void CloneSelected_Actors(bool bSelectNewClones, bool bClearPreviousSelections);

	void CloneSelected_Components(bool bSelectNewClones, bool bClearPreviousSelections);

public:


	/**
	 * Select Component adds a given Component to a list of components that will be used for the Runtime Transforms
	 * @param Component The component to add to the list.
	 * @param bClearPreviousSelections Whether this Selection clears the previous selected Components (set false to allow multi selection)
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectComponent(class USceneComponent* Component, bool bClearPreviousSelections = true);

	/**
	 * Select Actor adds the Actor's Root Component to a list of components that will be used for the Runtime Transforms
	 * @param Actor The Actor whose Root Component will be added to the list.
	 * @param bClearPreviousSelections Whether this Selection clears the previous selected Components (set false to allow multi selection)
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectActor(AActor* Actor, bool bClearPreviousSelections = true);

	/**
	 * Selects all the Components in given list.
	 * @see SelectComponent func
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectMultipleComponents(const TArray<class USceneComponent*>& Components, bool bClearPreviousSelections = true);

	/**
	 * Selects all the Root Components of the Actors in given list.
	 * @see SelectActor func
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectMultipleActors(const TArray<AActor*>& Actors, bool bClearPreviousSelections = true);

	/**
	 * Deselects a given Component, if found on the list.
	 * @param Component the Component to deselect
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void DeselectComponent(class USceneComponent* Component);

	/**
	 * Deselects a given Actor's Root Component, if found on the list.
	 * @param Actor whose RootComponent to deselect
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void DeselectActor(AActor* Actor);

	/*
	* Deselects all the Selected Components that are in the list.

	* @param bDestroyComponents - whether to Deselect all Components and Destroy them!

	* @return the list of components that were Deselected. (list will be empty if bDestroyComponents is true)
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	TArray<class USceneComponent*> DeselectAll(bool bDestroyDeselected = false);

private:

	/*
	The core functionality, but can be called by Selection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	void SelectComponent_Internal(class USceneComponent* Component);

	/*
	The core functionality, but can be called by Selection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	void DeselectComponent_Internal(class USceneComponent* Component);
	void DeselectComponentAtIndex_Internal(class USceneComponent* Component, int32 Index);

	/**
	 * Creates / Replaces Gizmo with the Current Transformation.
	 * It destroys any current active gizmo to replace it.
	*/
	void SetGizmo();

	/**
	 * Updates the Gizmo Placement (Position)
	 * Called when an object was selected, deselected
	*/
	void UpdateGizmoPlacement();

	//Gets the respective assigned class for a given TransformationType
	UClass* GetGizmoClass_ServerInternal(ETransformationType TransformationType) const;

	//Resets the transform accumulated (if Snapping is disabled, there shouldn't be any accumulated transform)
	void ResetAccumulatedTransform();

private:

	//The player controller whose Mouse is to be used for the Transformations
	APlayerController* PlayerController;

	//The Current Space being used, whether it is Local or World.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	ESpaceType CurrentSpaceType;

	//The Transform Accumulated for Snapping
	FTransform AccumulatedSnappedDeltaTransform;

	//The Transform Accumulated on Clients to afterwards Send to Servers when Transformation
	//has ended
	FTransform AccumulatedNetDeltaTransform;
	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATranslationGizmo> TranslationGizmoClass;

	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ARotationGizmo> RotationGizmoClass;

	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AScaleGizmo> ScaleGizmoClass;

	UPROPERTY()
	TWeakObjectPtr<class ABaseGizmo> Gizmo;

	// Tell which Domain is Selected. If NONE, then that means that there is no Selected Objects, or
	// that the Gizmo has not been hit yet.
	UPROPERTY(replicated)
	ETransformationDomain CurrentDomain;

	//Tell where the Gizmo should be placed when multiple objects are selected
	UPROPERTY(replicated, EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	EGizmoPlacement GizmoPlacement;

	// Var that tells which is the Current Transformation taking place
	UPROPERTY(replicated, EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	ETransformationType CurrentTransformation;

	/**
	 * Array storing Selected Components. Although a quick O(1) removal is needed (like a Set),
	 * it is Crucial that we maintain the order of the elements as they were selected
	 */
	TArray<FSelectableComponent> SelectedComponents;

	/*
	* Map storing the Snap values for each transformation
	* bSnappingEnabled must be true AND, the value for the current transform MUST NOT be 0 for these values to take effect.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TMap<ETransformationType, float> SnappingValues;

	/**
	* Whether Snapping an Object for each Transformation is enabled or not.
	* SnappingValue for each Transformation must also NOT be zero for it to work 
	* (if, snapping value is 0 for a transformation, no snapping will take place)

	* @see SetSnappingValue function & SnappingValues Map var
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TMap<ETransformationType, bool> SnappingEnabled;


	/**
	* Whether to Force Mobility on items that are not Moveable
	* if true, Mobility on Components will be changed to Moveable (WARNING: does not set it back to its original mobility!)
	* if false, no movement transformations will be attempted on Static/Stationary Components

	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bForceMobility;

	/*
	 * This property only matters when multiple objects are selected.
	 * Whether multiple objects should rotate on their local axes (true) or on the axes the Gizmo is in (false)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bRotateOnLocalAxis;

	/**
	 * Whether to Apply the Transforms to objects that Implement the UFocusable Interface.
	 * if True, the Transforms will be applied.
	 * if False, the Transforms will not be applied.

	 * IN BOTH Situations, the UFocusable Objects have IFocusable::OnNewDeltaTransformation called.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bTransformUFocusableObjects;

	//Property that checks whether a CLICK on an already selected object should deselect the object or not.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bToggleSelectedInMultiSelection;

	/*
	 * Property that checks whether Components are considered in trace 
	 or the Actors are.
	 * This property affects how Cloning, Tracing is done and Interface checking is done
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bComponentBased;
};
