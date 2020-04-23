// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FocusableObject.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFocusableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RUNTIMETRANSFORMER_API IFocusableObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Focusable")
	void Focus();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Focusable")
	void Unfocus();

	/* The Delta Transform in World Space (Local for Scaling) that has been calculated for a Selected Focusable Object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Focusable")
	void OnNewDeltaTransformation(const FTransform& DeltaTransform);

};
