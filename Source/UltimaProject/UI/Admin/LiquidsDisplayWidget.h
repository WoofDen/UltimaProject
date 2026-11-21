#pragma once

#include "Blueprint/UserWidget.h"
#include "LiquidsDisplayWidget.generated.h"

class SLiquidsDisplay;
class UContainerComponent;

/**
 * UMG wrapper around SLiquidsDisplay. Use ShowLiquids() to display the panel above the
 * container's owner actor; the panel follows the actor in the viewport every tick.
 */
UCLASS()
class ULiquidsDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Creates (or returns the already existing) liquids panel for the container and shows it slightly above
	 * the container owner's actor. Local player 0 is used as the widget owner.
	 * @param WorldZOffset extra height (in world units) above the top of the owner's bounds
	 */
	UFUNCTION(BlueprintCallable, Category = "Containers|UI")
	static ULiquidsDisplayWidget* ShowLiquids(UContainerComponent* Container, float WorldZOffset = 30.f);

	UFUNCTION(BlueprintCallable, Category = "Containers|UI")
	static void HideLiquids(UContainerComponent* Container);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

private:
	/** Projects the owner to the screen and moves the widget. Returns false if the widget should not be shown. */
	bool UpdateScreenPosition();

	TWeakObjectPtr<UContainerComponent> Container;
	float WorldZOffset = 30.f;

	TSharedPtr<SLiquidsDisplay> SlateWidget;

	static TMap<TWeakObjectPtr<UContainerComponent>, TWeakObjectPtr<ULiquidsDisplayWidget>> ActiveWidgets;
};
