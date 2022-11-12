// Alexander (AgitoReiKen) Moskalenko (C) 2022
#pragma once
#include "BlueprintConnectionDrawingPolicy.h"
#include "ConnectionDrawingPolicy.h"

#include "UObject/ConstructorHelpers.h"
struct FRestyleConnectionParams
{
	FGeometry Start;
	FGeometry End;
	FGeometry StartNodeGeometry;
	FGeometry EndNodeGeometry;

	FRestyleConnectionParams(const FGeometry& Start, const FGeometry& End, const FGeometry& StartNodeGeometry,
		const FGeometry& EndNodeGeometry)
		: Start(Start),
		  End(End),
		  StartNodeGeometry(StartNodeGeometry),
		  EndNodeGeometry(EndNodeGeometry)
	{
	}

	FRestyleConnectionParams(const FRestyleConnectionParams& Other)
		: Start(Other.Start),
		  End(Other.End),
		  StartNodeGeometry(Other.StartNodeGeometry),
		  EndNodeGeometry(Other.EndNodeGeometry)
	{
	}

	FRestyleConnectionParams(FRestyleConnectionParams&& Other) noexcept
		: Start(std::move(Other.Start)),
		  End(std::move(Other.End)),
		  StartNodeGeometry(std::move(Other.StartNodeGeometry)),
		  EndNodeGeometry(std::move(Other.EndNodeGeometry))
	{
	}

	FRestyleConnectionParams& operator=(const FRestyleConnectionParams& Other)
	{
		if (this == &Other)
			return *this;
		Start = Other.Start;
		End = Other.End;
		StartNodeGeometry = Other.StartNodeGeometry;
		EndNodeGeometry = Other.EndNodeGeometry;
		return *this;
	}

	FRestyleConnectionParams& operator=(FRestyleConnectionParams&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		Start = std::move(Other.Start);
		End = std::move(Other.End);
		StartNodeGeometry = std::move(Other.StartNodeGeometry);
		EndNodeGeometry = std::move(Other.EndNodeGeometry);
		return *this;
	}
};
/*
 * @todo Idea for caching. Make static class and pass Output pin / Input pin / StartGeom / EndGeom
 */
class FDefaultConnectionDrawingPolicy : public FKismetConnectionDrawingPolicy
{
public:

	FDefaultConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
		const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);
  
	void DrawConnection(const FRestyleConnectionParams& Params, const FConnectionParams& WireParams);
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint,
		const FVector2D& EndPoint, UEdGraphPin* Pin) override;
protected:

	virtual void DrawPinGeometries(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
		FArrangedChildren& ArrangedNodes) override;
	void UpdateSplineHover(const TArray<FVector2f>& Points, const FConnectionParams& Params, float ZoomValue);
	FGeometry GetNodeGeometryByPinWidget(SGraphPin& PinWidget, const FArrangedChildren& ArrangedNodes);
public: 

	TSharedPtr<class FPathDrawingSlateElement> LineDrawer;
};