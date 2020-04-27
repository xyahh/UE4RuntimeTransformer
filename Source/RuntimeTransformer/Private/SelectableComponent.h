// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
/**
 * 
 */
struct FSelectableComponent
{

	FSelectableComponent(class USceneComponent* Component = nullptr);
	~FSelectableComponent();

	void SetTransform(const FTransform& Transform, bool bTransformUFocusableObject, bool bComponentBase);

	class UObject* GetUFocusable(bool bComponentBase) const;

	void Select(bool bComponentBase, bool* bImplementsUFocusable = nullptr);
	void Deselect(bool bComponentBase, bool* bImplementsUFocusable = nullptr);

	bool operator==(const FSelectableComponent& other) const { return Component == other.Component; }
	
/* Vars */

	class USceneComponent* Component;
};
