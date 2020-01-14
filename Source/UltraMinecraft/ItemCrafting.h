// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemCrafting.generated.h"

USTRUCT(BlueprintType)
struct FItemCrafting : public FTableRowBase
{
	GENERATED_BODY()
	
	/** Full Path of Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Return;

	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item1;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item2;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item3;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item4;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item5;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item6;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item7;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item8;
	UPROPERTY(BlueprintReadWrite, Category = "GO")
	FString Item9;
};
