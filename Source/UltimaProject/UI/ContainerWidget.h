// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Game includes
#include "UltimaProject/Items/Containers/ContainerComponent.h"

// Engine includes
#include "Blueprint/UserWidget.h"
#include "ContainerWidget.generated.h"

/**
 * A container widget provides access to its items.
 */
UCLASS(Abstract)
class ULTIMAPROJECT_API UContainerWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// UUserWidget
	virtual void NativeDestruct() override;
	// ~UUserWidget
	
	
	UFUNCTION()
	void OnContainerItemsChange();
	
	// Called on client & server
	UFUNCTION(BlueprintImplementableEvent)
	void HandleContainerItemsChange();
	
public:
	TWeakObjectPtr<UContainerComponent> ContainerComponent;

	void SetContainerComponent(UContainerComponent* InContainerComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void OnContainerInitialized();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UContainerComponent* GetContainerComponent() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TScriptInterface<IContainerInterface> GetContainerInterface() const;
};
