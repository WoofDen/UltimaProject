// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CursorWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMAPROJECT_API UCursorWidget : public UUserWidget
{
	GENERATED_BODY()
	
	bool bIsOverInteractable = false;
	
	void UpdateCursor();
	
protected:
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CursorImage;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UObject> DefaultTextureObject;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UObject> InteractTextureObject;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UObject> TargetTextureObject;
	
public:
	void SetInteractHover(bool Value);
};
