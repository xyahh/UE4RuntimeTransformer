// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "RuntimeTransformer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRuntimeTransformer, Log, All);

UENUM(BlueprintType)
enum class ETransformationType : uint8
{
	TT_NoTransform			UMETA(DisplayName = "None"),
	TT_Translation			UMETA(DisplayName = "Translation"),
	TT_Rotation				UMETA(DisplayName = "Rotation"),
	TT_Scale				UMETA(DisplayName = "Scale")
};

UENUM(BlueprintType)
enum class ESpaceType : uint8
{
	ST_None				UMETA(DisplayName = "None"),
	ST_Local			UMETA(DisplayName = "Local Space"),
	ST_World			UMETA(DisplayName = "World Space"),
};

UENUM(BlueprintType) 
enum class ETransformationDomain : uint8
{
	TD_None				UMETA(DisplayName = "None"),

	TD_X_Axis			UMETA(DisplayName = "X Axis"),
	TD_Y_Axis			UMETA(DisplayName = "Y Axis"),
	TD_Z_Axis			UMETA(DisplayName = "Z Axis"),

	TD_XY_Plane			UMETA(DisplayName = "XY Plane"),
	TD_YZ_Plane			UMETA(DisplayName = "YZ Plane"),
	TD_XZ_Plane			UMETA(DisplayName = "XZ Plane"),

	TD_XYZ				UMETA(DisplayName = "XYZ"),

};

class FRuntimeTransformerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
