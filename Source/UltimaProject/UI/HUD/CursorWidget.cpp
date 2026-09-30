// Fill out your copyright notice in the Description page of Project Settings.

#include "CursorWidget.h"
#include "UltimaProject/Common/Macro.h"

void UCursorWidget::UpdateCursor()
{
	NULLCHECK(CursorImage);

	if (bIsOverInteractable)
	{
		CursorImage->SetBrushResourceObject(DefaultTextureObject);
	}
	else
	{
		CursorImage->SetBrushResourceObject(InteractTextureObject);
	}
}

void UCursorWidget::SetInteractHover(bool Value)
{
	bIsOverInteractable = Value;
	UpdateCursor();
}
