// Alexander (AgitoReiKen) Moskalenko (C) 2022
#include "DefaultAnimGraphConnectionDrawingPolicy.h"
#include "AnimationGraphSchema.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AnimGraphNode_Base.h"
#include "AnimGraphAttributes.h"

#include "Themes/Default/WireRestyleDefault.h"
//////////////////////////////////////////////
// FDefaultAnimGraphConnectionDrawingPolicy

FDefaultAnimGraphConnectionDrawingPolicy::FDefaultAnimGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FDefaultConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj)
{
}

bool FDefaultAnimGraphConnectionDrawingPolicy::TreatWireAsExecutionPin(UEdGraphPin* InputPin, UEdGraphPin* OutputPin) const
{
	const UAnimationGraphSchema* Schema = GetDefault<UAnimationGraphSchema>();

	return (InputPin != NULL) && (Schema->IsPosePin(OutputPin->PinType));
}

void FDefaultAnimGraphConnectionDrawingPolicy::BuildExecutionRoadmap()
{
	if(UAnimBlueprint* TargetBP = Cast<UAnimBlueprint>(FBlueprintEditorUtils::FindBlueprintForGraph(GraphObj)))
	{
		UAnimBlueprintGeneratedClass* AnimBlueprintClass = (UAnimBlueprintGeneratedClass*)(*(TargetBP->GeneratedClass));

		if (TargetBP->GetObjectBeingDebugged() == NULL)
		{
			return;
		}
		
		FAnimBlueprintDebugData& DebugInfo = AnimBlueprintClass->GetAnimBlueprintDebugData();
		for (auto VisitIt = DebugInfo.UpdatedNodesThisFrame.CreateIterator(); VisitIt; ++VisitIt)
		{
			const FAnimBlueprintDebugData::FNodeVisit& VisitRecord = *VisitIt;

			const int32 NumAnimNodeProperties = AnimBlueprintClass->GetAnimNodeProperties().Num();
			if ((VisitRecord.SourceID >= 0) && (VisitRecord.SourceID < NumAnimNodeProperties) && (VisitRecord.TargetID >= 0) && (VisitRecord.TargetID < NumAnimNodeProperties))
			{
				const int32 ReverseSourceID = NumAnimNodeProperties - 1 - VisitRecord.SourceID;
				const int32 ReverseTargetID = NumAnimNodeProperties - 1 - VisitRecord.TargetID;
				
				if (const UAnimGraphNode_Base* SourceNode = Cast<const UAnimGraphNode_Base>(DebugInfo.NodePropertyIndexToNodeMap.FindRef(ReverseSourceID)))
				{
					if (const UAnimGraphNode_Base* TargetNode = Cast<const UAnimGraphNode_Base>(DebugInfo.NodePropertyIndexToNodeMap.FindRef(ReverseTargetID)))
					{
						UEdGraphPin* PoseNet = NULL;

						UAnimationGraphSchema const* AnimSchema = GetDefault<UAnimationGraphSchema>();
						for (int32 PinIndex = 0; PinIndex < TargetNode->Pins.Num(); ++PinIndex)
						{
							UEdGraphPin* Pin = TargetNode->Pins[PinIndex];
							check(Pin);
							if (AnimSchema->IsPosePin(Pin->PinType) && (Pin->Direction == EGPD_Output))
							{
								PoseNet = Pin;
								break;
							}
						}

						if (PoseNet != NULL)
						{
							//@TODO: Extend the rendering code to allow using the recorded blend weight instead of faked exec times
							FExecPairingMap& Predecessors = PredecessorPins.FindOrAdd((UEdGraphNode*)SourceNode);
							FTimePair& Timings = Predecessors.FindOrAdd(PoseNet);
							Timings.PredExecTime = 0.0;
							Timings.ThisExecTime = FMath::Clamp(VisitRecord.Weight, 0.f, 1.f);
						}
					}
				}
			}
		}
	}
}

void FDefaultAnimGraphConnectionDrawingPolicy::DrawRestyleConnection(const FRestyleConnectionParams& Params,
	const FConnectionParams& WireParams, TArray<FVector2f>* InPoints)
{
	bool bCompositeWire = false;

	// Pose pins display attribute links
	if (WireParams.AssociatedPin1 && WireParams.AssociatedPin2)
	{
		if (UAnimationGraphSchema::IsPosePin(WireParams.AssociatedPin1->PinType))
		{
			// If either pin connects to a re-route (knot) node, traverse the link until a relevant pin is found
			UEdGraphPin* UsePin1 = WireParams.AssociatedPin1;
			if (WireParams.AssociatedPin1->GetOwningNode()->IsA<UK2Node_Knot>())
			{
				UsePin1 = FBlueprintEditorUtils::FindFirstCompilerRelevantLinkedPin(WireParams.AssociatedPin1);
			}

			UEdGraphPin* UsePin2 = WireParams.AssociatedPin2;
			if (WireParams.AssociatedPin2->GetOwningNode()->IsA<UK2Node_Knot>())
			{
				UsePin2 = FBlueprintEditorUtils::FindFirstCompilerRelevantLinkedPin(WireParams.AssociatedPin2);
			}

			if (UsePin1 && UsePin2)
			{
				UAnimGraphNode_Base* Node1 = Cast<UAnimGraphNode_Base>(UsePin1->GetOwningNode());
				UAnimGraphNode_Base* Node2 = Cast<UAnimGraphNode_Base>(UsePin2->GetOwningNode());
				if (Node1 && Node2)
				{
					const TArrayView<const SDefault_GraphPinPose::FAttributeInfo>& AdditionalAttributeInfo = PinAttributes.FindRef(WireParams.AssociatedPin1);

					if (AdditionalAttributeInfo.Num() > 0)
					{

						if (WireParams.WireThickness < 0 || !WireParams.AssociatedPin1 || !WireParams.AssociatedPin2) return;

						TArray<FVector2f> Points = MakePathPoints(Params, WireParams);
						TArray<FVector2f> _Points = Points;
						FConnectionParams _WireParams = WireParams;
						_WireParams.WireThickness *= UWireRestyleSettings::Get()->AttributeWireThicknessMultiplier;
						_WireParams.WireColor = UWireRestyleSettings::Get()->AttributeWireColor.Get();  FLinearColor(1.0, 0.5, 0.0, 1.0);
						_WireParams.bDrawBubbles = _WireParams.bDrawBubbles ? !UWireRestyleSettings::Get()->AttributeDisableBubbles : false;
						FDefaultConnectionDrawingPolicy::DrawRestyleConnection(Params, WireParams, &Points);
						FDefaultConnectionDrawingPolicy::DrawRestyleConnection(Params, _WireParams, &_Points);
						return;
						//const FSlateBrush* AttributeWidget = FAppStyle::GetBrush(FWireRestyleStyles::Square);
						//float TotalLength = 0.f;
						//for (int i = 1; i < Points.Num(); i++)
						//{
						//	const auto& A = Points[i - 1];
						//	const auto& B = Points[i];
						//	double Length = FVector2f::Distance(A, B);
						//	TotalLength += Length;
						//}
						//int Instances = AdditionalAttributeInfo.Num();
						//float MaxStep = Zoomed(32.f);
						//float Step = FMath::Min(TotalLength / Instances, MaxStep);

						//const FVector2f WidgetSize = FVector2f(Zoomed(WireParams.WireThickness * 4));
						//const FVector2f WidgetHalfSize = WidgetSize * 0.5;
						//float fWidgetSize = WidgetSize.X;
						//int Drawn = 0;
						//float HalfLength = TotalLength * .5f;
						//float EstimatedDrawLength = ((Step * Instances - 1) + (fWidgetSize * Instances));
						//float LengthOffset = HalfLength - EstimatedDrawLength * .5f;
						//while (Drawn != Instances)
						//{
						//	float DrawLength = LengthOffset + (Drawn * Step);
						//	float CurrentLength = 0.f;
						//	for (int i = 1; i < Points.Num(); i++)
						//	{
						//		const auto& A = Points[i - 1];
						//		const auto& B = Points[i];
						//		double Length = FVector2f::Distance(A, B);
						//		auto Normal = (B - A).GetSafeNormal();
						//		if (DrawLength < CurrentLength + Length)
						//		{
						//			float Offset = DrawLength - CurrentLength;
						//			FVector2f WidgetPos = A + (Normal * Offset) - WidgetHalfSize; // *BubbleOffset);
						//			FSlateDrawElement::MakeBox(
						//				DrawElementsList,
						//				ArrowLayerID,
						//				FPaintGeometry(FVector2D(WidgetPos.X, WidgetPos.Y), 
						//					FVector2D(WidgetSize.X, WidgetSize.Y), ZoomFactor),
						//				AttributeWidget,
						//				ESlateDrawEffect::None,
						//				AdditionalAttributeInfo[Drawn].Color
						//			);
						//			Drawn++;
						//			break;
						//		}

						//		CurrentLength += Length;
						//		if (i + 1 == Points.Num())
						//		{
						//			DrawLength -= CurrentLength;
						//			i = 1;
						//		}
						//	}
						//}
						//const float MaxAttributeWireThickness = 3.0f;
						//const float MinAttributeWireThickness = 1.0f;
						//const float MaxWireGap = 2.0f;
						//const float MinWireGap = 0.5f;

						//// 0.375f is the zoom level before the 'low LOD' cutoff
						//const float ZoomLevelAlpha = FMath::GetMappedRangeValueClamped(TRange<float>(0.375f, 1.0f), TRange<float>(0.0f, 1.0f), PanelZoom);
						//const float AttributeWireThickness = FMath::Lerp(MinAttributeWireThickness, MaxAttributeWireThickness, ZoomLevelAlpha);
						//const float WireGap = FMath::Lerp(MinWireGap, MaxWireGap, ZoomLevelAlpha);

						//const FVector2D& P0 = FVector2D(Points[0].X, Points[0].Y);
						//const FVector2D& P1 = FVector2D(Points.Last().X, Points.Last().Y);

						//const FVector2D SplineTangent = ComputeSplineTangent(P0, P1);
						//const FVector2D P0Tangent = (WireParams.StartDirection == EGPD_Output) ? SplineTangent : -SplineTangent;
						//const FVector2D P1Tangent = (WireParams.EndDirection == EGPD_Input) ? SplineTangent : -SplineTangent;

						//bCompositeWire = true;

						//float TotalThickness = WireParams.WireThickness;

						//static TArray<float> CachedWireThicknesses;
						//check(CachedWireThicknesses.Num() == 0);	// Cant be called recursively or on multiple threads
						//CachedWireThicknesses.SetNumZeroed(AdditionalAttributeInfo.Num());

						//for (int32 AttributeInfoIndex = 0; AttributeInfoIndex < AdditionalAttributeInfo.Num(); ++AttributeInfoIndex)
						//{
						//	const SDefault_GraphPinPose::FAttributeInfo& AttributeInfo = AdditionalAttributeInfo[AttributeInfoIndex];

						//	float WireThickness = 0.0f;
						//	switch (AttributeInfo.Blend)
						//	{
						//	case EAnimGraphAttributeBlend::Blendable:
						//	{
						//		if (UAnimBlueprint* AnimBlueprint = Cast<UAnimBlueprint>(FBlueprintEditorUtils::FindBlueprintForGraph(GraphObj)))
						//		{
						//			UAnimBlueprintGeneratedClass* AnimBlueprintClass = (UAnimBlueprintGeneratedClass*)(*(AnimBlueprint->GeneratedClass));
						//			int32 SourceNodeId = AnimBlueprintClass->GetNodeIndexFromGuid(Node1->NodeGuid);
						//			int32 TargetNodeId = AnimBlueprintClass->GetNodeIndexFromGuid(Node2->NodeGuid);
						//			if (SourceNodeId != INDEX_NONE && TargetNodeId != INDEX_NONE)
						//			{
						//				const TArray<FAnimBlueprintDebugData::FAttributeRecord>* LinkAttributes = AnimBlueprintClass->GetAnimBlueprintDebugData().NodeOutputAttributesThisFrame.Find(SourceNodeId);
						//				const bool bAttributeUsedInLink = LinkAttributes && LinkAttributes->ContainsByPredicate(
						//					[&AttributeInfo, TargetNodeId](const FAnimBlueprintDebugData::FAttributeRecord& InRecord)
						//					{
						//						return InRecord.Attribute == AttributeInfo.Attribute && InRecord.OtherNode == TargetNodeId;
						//					});


						//				WireThickness = bAttributeUsedInLink ? AttributeWireThickness : 0.0f;
						//			}
						//		}
						//		break;
						//	}
						//	case EAnimGraphAttributeBlend::NonBlendable:
						//		WireThickness = AttributeWireThickness;
						//		break;
						//	}

						//	CachedWireThicknesses[AttributeInfoIndex] = WireThickness;
						//	TotalThickness += WireThickness != 0.0f ? (WireThickness + WireGap) : 0.0f;
						//}

						//const float InitialOffset = TotalThickness * 0.5f;
						//FVector2D SubWireP0 = P0;
						//SubWireP0.Y += InitialOffset;
						//FVector2D SubWireP1 = P1;
						//SubWireP1.Y += InitialOffset;
						//// Draw in reverse order to get pose wires appearing on top
						//for (int32 AttributeInfoIndex = AdditionalAttributeInfo.Num() - 1; AttributeInfoIndex >= 0; --AttributeInfoIndex)
						//{
						//	float Thickness = CachedWireThicknesses[AttributeInfoIndex];
						//	if (Thickness > 0.0f)
						//	{
						//		const SDefault_GraphPinPose::FAttributeInfo& AttributeInfo = AdditionalAttributeInfo[AttributeInfoIndex];
						//		FLinearColor Color = AttributeInfo.Color;

						//		if (HoveredPins.Num() > 0)
						//		{
						//			ApplyHoverDeemphasis(WireParams.AssociatedPin1, WireParams.AssociatedPin2, Thickness, Color);
						//		}

						//		SubWireP0.Y -= Thickness + WireGap;
						//		SubWireP1.Y -= Thickness + WireGap;

						//		// Draw the spline itself
						//		FSlateDrawElement::MakeDrawSpaceSpline(
						//			DrawElementsList,
						//			WireLayerID,
						//			SubWireP0, P0Tangent,
						//			SubWireP1, P1Tangent,
						//			Thickness,
						//			ESlateDrawEffect::None,
						//			Color
						//		);
						//	}
						//}

						//SubWireP0.Y -= WireParams.WireThickness + WireGap;
						//SubWireP1.Y -= WireParams.WireThickness + WireGap;

						 


						//FDefaultConnectionDrawingPolicy::DrawConnection(LayerId, SubWireP0, SubWireP1, Params);

						//CachedWireThicknesses.Reset();
					}
				}
			}
		}
	}

	if (!bCompositeWire)
	{
		//DefaultConnectionDrawingPolicy::DrawConnection(LayerId, Start, End, Params);
		FDefaultConnectionDrawingPolicy::DrawRestyleConnection(Params, WireParams);
	}
}

void FDefaultAnimGraphConnectionDrawingPolicy::BuildPinToPinWidgetMap(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries)
{
	FDefaultConnectionDrawingPolicy::BuildPinToPinWidgetMap(InPinGeometries);

	// Cache additional attributes
	PinAttributes.Reset();
	bool bFoundPanelZoom = false;
	PanelZoom = 1.0f;

	for(const TPair<UEdGraphPin*, TSharedPtr<SGraphPin>>& PinWidgetPair : PinToPinWidgetMap)
	{
		if(PinWidgetPair.Key->Direction == EGPD_Output && UAnimationGraphSchema::IsPosePin(PinWidgetPair.Key->PinType) && PinWidgetPair.Key->GetOwningNode()->IsA<UAnimGraphNode_Base>())
		{
			// Pose pins are assumed to be SDefault_GraphPinPose widgets here
			check(PinWidgetPair.Value->GetType() == TEXT("SDefault_GraphPinPose"));
			TSharedPtr<SDefault_GraphPinPose> PosePin = StaticCastSharedPtr<SDefault_GraphPinPose>(PinWidgetPair.Value);

			PinAttributes.Add(PinWidgetPair.Key, PosePin->GetAttributeInfo());

			if(!bFoundPanelZoom)
			{
				PanelZoom = PosePin->GetZoomAmount();
				bFoundPanelZoom = true;
			}
		}
	}
}

void FDefaultAnimGraphConnectionDrawingPolicy::DetermineStyleOfExecWire(float& Thickness, FLinearColor& WireColor, bool& bDrawBubbles, const FTimePair& Times)
{
	// It's a followed link, make it strong and yellowish but fading over time
	const double BlendWeight = Times.ThisExecTime;

	const float HeavyBlendThickness = AttackWireThickness;
	const float LightBlendThickness = SustainWireThickness;

	Thickness = FMath::Lerp<float>(LightBlendThickness, HeavyBlendThickness, BlendWeight);
	WireColor = WireColor * (BlendWeight * 0.5f + 0.5f);//FMath::Lerp<FLinearColor>(SustainColor, AttackColor, BlendWeight);

	bDrawBubbles = true;
}
 

void FDefaultAnimGraphConnectionDrawingPolicy::ApplyHoverDeemphasis(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, float& Thickness, FLinearColor& WireColor)
{
	// Remove the thickness increase on hover
	// Why?
	/* ReleaseWireThickness used as Default */
	bool bDebugging = Thickness > ReleaseWireThickness;
	if (!bDebugging) {
		FDefaultConnectionDrawingPolicy::ApplyHoverDeemphasis(OutputPin, InputPin, Thickness, WireColor);
	}
}