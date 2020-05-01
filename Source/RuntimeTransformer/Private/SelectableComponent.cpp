// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "SelectableComponent.h"
#include "RuntimeTransformer/Public/FocusableObject.h"
#include "RuntimeTransformer/Public/TransformerPawn.h"
#include "Components/SceneComponent.h"

FSelectableComponent::FSelectableComponent(class USceneComponent* Component_) 
	: Component(Component_)
{
}

FSelectableComponent::~FSelectableComponent()
{
}

void FSelectableComponent::SetTransform(const FTransform& Transform, bool bTransformUFocusableObjects, bool bComponentBase)
{
	if (!Component) return;

	if (UObject* focusableObject = GetUFocusable(bComponentBase))
	{
		IFocusableObject::Execute_OnNewTransformation(focusableObject, Component, Transform, bComponentBase);
		if (bTransformUFocusableObjects)
			Component->SetWorldTransform(Transform);
	}
	else
		Component->SetWorldTransform(Transform);
}

UObject* FSelectableComponent::GetUFocusable(bool bComponentBase) const
{
	if (!Component) return nullptr;

	if (bComponentBase)
		return (Component->Implements<UFocusableObject>()) ? Component : nullptr;
	else if (AActor* ComponentOwner = Component->GetOwner())
		return (ComponentOwner->Implements<UFocusableObject>()) ? ComponentOwner : nullptr;

	return nullptr;
}

void FSelectableComponent::Select(bool bComponentBase, bool* bImplementsUFocusable)
{
	UObject* focusableObject = GetUFocusable(bComponentBase);
	if (focusableObject)
		IFocusableObject::Execute_Focus(focusableObject, Component, bComponentBase);
	if (bImplementsUFocusable)
		*bImplementsUFocusable = !!focusableObject;
}

void FSelectableComponent::Deselect(bool bComponentBase, bool* bImplementsUFocusable)
{
	UObject* focusableObject = GetUFocusable(bComponentBase);
	if (focusableObject)
		IFocusableObject::Execute_Unfocus(focusableObject, Component, bComponentBase);
	if (bImplementsUFocusable)
		*bImplementsUFocusable = !!focusableObject;
}
