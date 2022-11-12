// Alexander (AgitoReiKen) Moskalenko (C) 2022

#pragma once

#include "SDefault_NodeTitle.h"

#include "KismetNodes/SGraphNodeK2Default.h"
 
class UK2Node;
class UDefaultThemeSettings;
class UNodeRestyleSettings;
class SDefault_GraphNodeK2Base : public SGraphNodeK2Base
{

public:
	SDefault_GraphNodeK2Base()
	{
		CachedNoDrawBrush = FAppStyle::Get().GetBrush(FNodeRestyleStyles::NoDrawBrush);
		CachedOutlineWidth = .0f;
		CachedState = EDTGraphNodeState::Num;
		ErrorInfoType = EDTNodeReportType::Num;
	}

	virtual bool IsInvalid() const;
	virtual bool IsDisabled() const;
	/** @return True if state changed*/
	bool UpdateState(bool bSelected = false) const;
	 

	void UpdateErrorInfo_New();
	void SetupErrorReporting_New();
	TSharedPtr<SWidget> GetEnabledStateWidget_New();
	FMargin GetEnabledStateWidgetPadding() const;
	FReply OnAdvancedDisplayClicked();
	/** Create widget to show/hide advanced pins */
	virtual void CreateAdvancedViewArrowNew(TSharedPtr<SVerticalBox> MainBox);
 
	virtual void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
	void SetupRenderOpacity();
protected:
	const FSlateBrush* CachedNoDrawBrush;
	TSharedRef<SWidget> AddPinButtonContent_New(FText PinText, FText PinTooltipText, bool bRightSide = true, FString DocumentationExcerpt = FString(), TSharedPtr<SToolTip> CustomTooltip = NULL);
	FMargin EnabledStateWidgetAdditionalPadding;
	mutable float CachedOutlineWidth;
	mutable EDTGraphNodeState CachedState;
	EDTNodeReportType ErrorInfoType;
	 
	//InlineEditableText -> MainText
};