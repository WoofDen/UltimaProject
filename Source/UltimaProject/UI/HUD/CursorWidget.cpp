// Fill out your copyright notice in the Description page of Project Settings.

#include "CursorWidget.h"
#include "UltimaProject/Common/Macro.h"

void UCursorWidget::UpdateCursor()
{
	NULLCHECK(CursorImage);
	
	// Targeting has prio over other cursors
	if (bIsTargetMode)
	{
		CursorImage->SetBrushResourceObject(TargetTextureObject);
		return;
	}

	if (bIsOverInteractable)
	{
		CursorImage->SetBrushResourceObject(InteractTextureObject);
	}
	else
	{
		CursorImage->SetBrushResourceObject(DefaultTextureObject);
	}
}

void UCursorWidget::SetInteractHover(bool Value)
{
	bIsOverInteractable = Value;
	UpdateCursor();
}

void UCursorWidget::SetTargetMode(bool Value)
{
	bIsTargetMode = Value;
	UpdateCursor();
}
