// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeTransformer.h"
#include "TransformerActor.generated.h"

UENUM(BlueprintType)
enum EGizmoPlacement
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

protected:

	virtual void BeginPlay() override;

public:

	/*
	* This gets called everytime a Component / Actor is going to get added.
	* The default return is TRUE, but it can be overriden to check for additional things 
	* (e.g. checking if it implements an interface, has some property, etc)
	
	* @param OwnerActor: The Actor owning the Component Selected 
	* @param Component: The Component Selected (if it's an Actor Selected, this would be its RootComponent)

	* @return bool: Whether or not this Component should be added.
	*/
	UFUNCTION(BlueprintNativeEvent)
	bool ShouldSelect(AActor* OwnerActor, class USceneComponent* Component);

	//by default return true
	virtual bool ShouldSelect_Implementation(AActor* OwnerActor, class USceneComponent* Component) { return true; }

	/**
	 * Setting a Player Controller makes most functionality automatic, but
	 * The Player Controller is used to get the Mouse World Space Position & Direction for the given Transformations.

	 * If no Player Controller is set,  Tracing & UpdateTransform functions must be called manually.
	*/
	UFUNCTION(BlueprintCallable)
	void SetPlayerController(class APlayerController* Controller);

	//Sets the Space of the Gizmo, whether its Local or World space.
	UFUNCTION(BlueprintCallable)
	void SetSpaceType(TEnumAsByte<ESpaceType> Type);

	/*
	 * Gets the Current Domain
	 * If it returns ETransformationDomain::TD_None, then that means
	 * there is no Transformation in Progress.
	*/
	UFUNCTION(BlueprintCallable)
	TEnumAsByte<ETransformationDomain> GetCurrentDomain(bool& TransformInProgress) const;


	/**
	 * Releases the Current Domain and sets it to NONE.
	 * Should be called when we are done transforming with the Gizmo.
	*/
	UFUNCTION(BlueprintCallable)
	void ReleaseDomain();


	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable)
	bool MouseTraceByObjectTypes(float TraceDistance, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param TraceChannel - The Ray Collision Channel to be Considered in the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable)
	bool MouseTraceByChannel(float TraceDistance, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param ProfileName - The Profile Name to be used during the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable)
	bool MouseTraceByProfile(float TraceDistance, const FName& ProfileName
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bClearPreviousSelections - If a selection happens, whether to unselect the previously selected components (false allows multi selection)
	 * @param bTraceComponent - Whether the trace looks for Actors hit, or Components hit
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable)
	bool TraceByObjectTypes(const FVector& StartLocation, const FVector& EndLocation
		, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

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
	UFUNCTION(BlueprintCallable)
	bool TraceByChannel(const FVector& StartLocation, const FVector& EndLocation
		, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);


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
	UFUNCTION(BlueprintCallable)
	bool TraceByProfile(const FVector& StartLocation, const FVector& EndLocation
		, const FName& ProfileName
		, TArray<AActor*> IgnoredActors
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

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
	UFUNCTION(BlueprintCallable)
	void UpdateTransform(const FVector& LookingVector, const FVector& RayOrigin, const FVector& RayDirection);



	/**
	 * Processes the OutHits generated by Tracing and Selects either a Gizmo (priority) or
	 * if no Gizmo is present in the trace, the first object hit is selected.
	 *
	 * This is already called by the RuntimeTransformer built-in Trace Functions,
	 * but can be called manually if you wish to provide your own list of Hit Results (e.g. tracing with different configuration/method)
	 *
	 * @param HitResults - a list of the FHitResults that were generated by LineTracing
	 * @param bClearPrevious Selections - whether we should clear the previously selected objects (only relevant when there is no Gizmo in hit list)
	 * @param bTraceComponent - whether we should get the HitComponent or not. If false, the HitActor will be used instead.
	*/
	UFUNCTION(BlueprintCallable)
	bool HandleTracedObjects(const TArray<FHitResult>& HitResults
		, bool bClearPreviousSelections = true, bool bTraceComponent = false);

protected:

	UFUNCTION(BlueprintNativeEvent)
	void OnGizmoStateChanged(ETransformationType GizmoType, bool bTransformInProgress, ETransformationDomain Domain);

	virtual void OnGizmoStateChanged_Implementation(ETransformationType GizmoType, bool bTransformInProgress, ETransformationDomain Domain)
	{
		//this should be overriden for custom logic
	}

	/*
	 * Called when a new Component has been Selected (Focused)
	 * or has been unselected (unfocused).

	 * This should be used if there needs to be logic applied to
	 * objects that do not implement the UFocusable interface.
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnComponentSelectionChange(class USceneComponent* Component, bool bSelected);

	virtual void OnComponentSelectionChange_Implementation(class USceneComponent* Component, bool bSelected)
	{
		//This should be overriden for custom logic
	}

public:


	UFUNCTION(BlueprintCallable)
	void SetTransformationType(TEnumAsByte<ETransformationType> TransformationType);
	
	/*
	 * Gets the list of Selected Components.

	 @return outComponentList - the List of Currently Selected Components
	 @return outGizmoPlacedComponent - the Component in the list that currently has the Gizmo attached
	*/
	UFUNCTION(BlueprintCallable)
	void GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
		, class USceneComponent*& outGizmoPlacedComponent) const;

	/*
	* Makes an exact copy of the Actors that are owners of the components and makes
	* a copy of them.
	
	* Take care of not spamming this :)

	* @param bSelectNewClones - whether to add the new clones to the Selection
	* @param bClearPreviousSelections Whether to clear the previous selected Components 
	*/
	UFUNCTION(BlueprintCallable)
	void CloneSelectedComponents(bool bSelectNewClones = true , bool bClearPreviousSelections = true);

	/**
	 * Select Component adds a given Component to a list of components that will be used for the Runtime Transforms
	 * @param Component The component to add to the list.
	 * @param bClearPreviousSelections Whether this Selection clears the previous selected Components (set false to allow multi selection)
	 */
	UFUNCTION(BlueprintCallable)
	void SelectComponent(class USceneComponent* Component, bool bClearPreviousSelections = true);

	/**
	 * Select Actor adds the Actor's Root Component to a list of components that will be used for the Runtime Transforms
	 * @param Actor The Actor whose Root Component will be added to the list.
	 * @param bClearPreviousSelections Whether this Selection clears the previous selected Components (set false to allow multi selection)
	 */
	UFUNCTION(BlueprintCallable)
	void SelectActor(AActor* Actor, bool bClearPreviousSelections = true);

	/**
	 * Selects all the Components in given list.
	 * @see SelectComponent func
	 */
	UFUNCTION(BlueprintCallable)
	void SelectMultipleComponents(const TArray<class USceneComponent*>& Components, bool bClearPreviousSelections = true);

	/**
	 * Selects all the Root Components of the Actors in given list.
	 * @see SelectActor func
	 */
	UFUNCTION(BlueprintCallable)
	void SelectMultipleActors(const TArray<AActor*>& Actors, bool bClearPreviousSelections = true);

	/**
	 * Deselects a given Component, if found on the list.
	 * @param Component the Component to deselect
	 */
	UFUNCTION(BlueprintCallable)
	void DeselectComponent(class USceneComponent* Component);

	/**
	 * Deselects a given Actor's Root Component, if found on the list.
	 * @param Actor whose RootComponent to deselect
	 */
	UFUNCTION(BlueprintCallable)
	void DeselectActor(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void DeselectAll();

private:

	/*
	The core functionality, but can be called by Selection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	UFUNCTION(BlueprintCallable)
	void SelectComponent_Internal(class USceneComponent* Component);

	/*
	The core functionality, but can be called by Deselection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	void DeselectComponent_Internal(class USceneComponent* Component);
	void DeselectComponentByIndex_Internal(class USceneComponent* Component, int32 Index);

	/* Interface Func calls */
	void CallFocus_Internal(class USceneComponent* Component);
	void CallUnfocus_Internal(class USceneComponent* Component);
	void CallOnNewDeltaTransformation_Internal(class USceneComponent* Component, const FTransform& DeltaTransform);

	//gets either the Parent, or Child (or Both) UObjects that Implement UFocusable
	TArray<UObject*> GetUFocusableObjects(class USceneComponent* Component) const;

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

	UClass* GetGizmoClass(TEnumAsByte<ETransformationType> TransformationType) const;


private:

	void CopyActorProperties(AActor* Source, AActor* Target);

	//The player controller whose Mouse is to be used for the Transformations
	APlayerController* PlayerController;

	//The Current Space being used, whether it is Local or World.
	TEnumAsByte<ESpaceType> CurrentSpaceType;

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
	 * Whether to Apply the Delta Transforms to objects that Implement the UFocusable Interface.
	 * if True, the Transforms will be applied.
	 * if False, the Transforms will not be applied.

	 * IN BOTH Situations, the UFocusable Objects have IFocusable::OnNewDeltaTransformation called.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bTransformUFocusableObjects;

	//Property that checks whether a CLICK on an already selected object should deselect the object or not.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bToggleSelectedInMultiSelection;

	UPROPERTY()
	TWeakObjectPtr<class ABaseGizmo> Gizmo;

	// Tell which Domain is Selected. If NONE, then that means that there is no Selected Objects, or
	// that the Gizmo has not been hit yet.
	TEnumAsByte<ETransformationDomain> CurrentDomain;

	//Tell where the Gizmo should be placed when multiple objects are selected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EGizmoPlacement> GizmoPlacement;

	// Var that tells which is the Current Transformation taking place
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETransformationType> CurrentTransformation;

	/**
	 * Array storing Selected Components. Although a quick O(1) removal is needed (like a Set),
	 * it is Crucial that we maintain the order of the elements as they were selected
	 */
	TArray<class USceneComponent*> SelectedComponents;
};
