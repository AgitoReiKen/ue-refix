// Alexander (AgitoReiKen) Moskalenko (C) 2022

#include "SDefault_GraphNodeK2Var.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "K2Node.h"
#include "K2Node_StructOperation.h"
#include "K2Node_StructMemberGet.h"
#include "K2Node_Literal.h"
#include "K2Node_StructMemberSet.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "SDefault_CommentBubble.h"

#include "Styling/SlateIconFinder.h"
#include "TutorialMetaData.h"

void SDefault_GraphNodeK2Var::Construct(const FArguments& InArgs, UK2Node* InNode)
{
	GraphNode = InNode;
	auto Style = UNodeRestyleSettings::Get();
	CachedState = IsInvalid() ? EDTGraphNodeState::Invalid : EDTGraphNodeState::Normal;
	CachedOutlineWidth = UDefaultThemeSettings::GetOutlineWidth(
		Style->VarNode.GetTypeData(GetNodeType()).GetState(CachedState).Body.Get().OutlineWidth
	);
	EnabledStateWidgetAdditionalPadding = UDefaultThemeSettings::GetMargin(Style->StateWidget.Padding);
	CachedVariableColor = GraphNode->GetNodeTitleColor();
	bVariableColorIsWhite = CachedVariableColor == FLinearColor::White;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

FSlateColor SDefault_GraphNodeK2Var::GetVariableColor() const
{
	return CachedVariableColor;
}

TSharedRef<SWidget> SDefault_GraphNodeK2Var::UpdateTitleWidget(FText InTitleText, TSharedPtr<SWidget> InTitleWidget,
                                                               EHorizontalAlignment& InOutTitleHAlign,
                                                               FMargin& InOutTitleMargin) const
{
	if (InTitleText.IsEmpty())
	{
		TitleTextBlock = SNullWidget::NullWidget;
	}

	if (!TitleTextBlock.IsValid())
	{
		TitleTextBlock = SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), FNodeRestyleStyles::VarNode_Title_Text)
			.Text(InTitleText);
	}

	return TitleTextBlock.ToSharedRef();
}

void SDefault_GraphNodeK2Var::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	SetupRenderOpacity();

	FText TitleText;
	bool bPadTitle = false;
	float HorizontalTitleMargin = 0.0f;
	float VerticalTitleMargin = 8.0f;
	 
	EHorizontalAlignment TitleHAlign = HAlign_Center;
	TSharedPtr<SWidget> TitleWidget;
	EDTVarType VarType = GetNodeType();
	const auto& VarNode = UNodeRestyleSettings::Get()->VarNode;
	FDTVarNodeState State = VarNode.GetTypeData(VarType).GetState(CachedState);
	float ContentSpacing = UDefaultThemeSettings::GetSpacing(VarNode.ContentSpacing);
	FMargin TitleMargin = UDefaultThemeSettings::GetMargin(VarNode.TitlePadding);
	FMargin ContentAreaMargin = UDefaultThemeSettings::GetMargin(VarNode.ContentAreaPadding); //FMargin(0.0f, 4.0f);
	if (GraphNode->IsA(UK2Node_VariableSet::StaticClass()))
	{
		UK2Node_VariableSet* SetNode = Cast<UK2Node_VariableSet>(GraphNode);
		if (SetNode->HasLocalRepNotify())
		{
			TitleText = NSLOCTEXT("GraphEditor", "VariableSetWithNotify", "SET w/ Notify");
		}
		else
		{
			TitleText = NSLOCTEXT("GraphEditor", "VariableSet", "SET");
		}
	}
	else if (UK2Node_StructOperation* StructOp = Cast<UK2Node_StructOperation>(GraphNode))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("VariableName"), StructOp->GetVarNameText());
		if (GraphNode->IsA(UK2Node_StructMemberGet::StaticClass()))
		{
			TitleText = FText::Format(NSLOCTEXT("GraphEditor", "StructMemberGet", "Get in {VariableName}"), Args);
		}
		else if (GraphNode->IsA(UK2Node_StructMemberSet::StaticClass()))
		{
			TitleText = FText::Format(NSLOCTEXT("GraphEditor", "StructMemberSet", "Set in {VariableName}"), Args);
		}
		else if (GraphNode->IsA(UK2Node_MakeStruct::StaticClass()))
		{
			TitleText = FText::Format(NSLOCTEXT("GraphEditor", "MakeStruct", "Make {VariableName}"), Args);
		}
		else
		{
			check(false);
		}
		bPadTitle = true;
		//HorizontalTitleMargin = 12.0f;
		//TitleMargin = FMargin(12.0f, VerticalTitleMargin);
	}
	else if (UK2Node_Literal* LiteralRef = Cast<UK2Node_Literal>(GraphNode))
	{
		FText SubTitleText;

		// Get the name of the level the object is in.
		if (AActor* Actor = Cast<AActor>(LiteralRef->GetObjectRef()))
		{
			FText LevelName;

			if (ULevel* Level = Actor->GetLevel())
			{
				if (Level->IsPersistentLevel())
				{
					LevelName = NSLOCTEXT("GraphEditor", "PersistentTag", "Persistent Level");
				}
				else
				{
					LevelName = FText::FromString(FPaths::GetCleanFilename(Actor->GetOutermost()->GetName()));
				}
			}
			SubTitleText = FText::Format(NSLOCTEXT("GraphEditor", "ActorRef", "from {0}"), LevelName);
		}

		TitleText = GraphNode->GetNodeTitle(ENodeTitleType::FullTitle);

		TitleHAlign = HAlign_Left;
		//TitleMargin = FMargin(12.0f, VerticalTitleMargin, 32.0f, 2.0f);

		TitleWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Left)
			  .VAlign(VAlign_Top)
			  .AutoWidth()
			[
				SNew(SImage)
				.Image(FSlateIconFinder::FindIconBrushForClass(
					LiteralRef->GetObjectRef() ? LiteralRef->GetObjectRef()->GetClass() : NULL))
			]

			+ SHorizontalBox::Slot()
			  .Padding(2.0f, 0.0f, 0.0f, 0.0f)
			  .HAlign(HAlign_Left)
			  .VAlign(VAlign_Top)
			  .AutoWidth()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				  .VAlign(VAlign_Top)
				  .AutoHeight()
				[
					SNew(STextBlock)
					.WrapTextAt(128.0f)
					.TextStyle(FEditorStyle::Get(), "Graph.Node.NodeTitle")
					.Text(TitleText)
				]

				+ SVerticalBox::Slot()
				  .VAlign(VAlign_Top)
				  .AutoHeight()
				[
					SNew(STextBlock)
					.Visibility(TitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
					.WrapTextAt(128.0f)
					.TextStyle(FEditorStyle::Get(), "Graph.Node.NodeTitleExtraLines")
					.Text(SubTitleText)
				]
			];
	}
	else if (UK2Node_VariableGet* VariableGet = Cast<UK2Node_VariableGet>(GraphNode))
	{
		if (!VariableGet->IsNodePure())
		{
			TitleText = NSLOCTEXT("GraphEditor", "VariableGet", "GET");
			//ContentAreaMargin.Top += 16;
		}
	}
	TitleWidget = UpdateTitleWidget(TitleText, TitleWidget, TitleHAlign, TitleMargin);

	SetupErrorReporting_New();

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);
	//             ________________
	//            | (>) L |  R (>) |
	//            | (>) E |  I (>) |
	//            | (>) F |  G (>) |
	//            | (>) T |  H (>) |
	//            |       |  T (>) |
	//            |_______|________|
	//
	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SOverlay)
				.AddMetaData<FGraphNodeMetaData>(TagMeta)
				+ SOverlay::Slot()
				[
					SAssignNew(VarNodeBody, SImage)
					.Image(FEditorStyle::GetBrush(FNodeRestyleStyles::VarNode_Body(VarType, CachedState)))
				]
				+ SOverlay::Slot()
				  .VAlign(VAlign_Top)
				  .HAlign(TitleHAlign)
				  .Padding(TitleMargin)
				[
					TitleWidget.ToSharedRef()
				]
				+ SOverlay::Slot()
				  .Padding(0)
				  .VAlign(VAlign_Center)
				  .Padding(ContentAreaMargin)
				[
					// NODE CONTENT AREA
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					  .HAlign(HAlign_Left)
					  .FillWidth(1.0f)
					  .Padding(0)
					[
						// LEFT
						SAssignNew(LeftNodeBox, SVerticalBox)
					]
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .HAlign(HAlign_Right)
					  .Padding(ContentSpacing, 0, 0, 0)
					[
						// RIGHT
						SAssignNew(RightNodeBox, SVerticalBox)
					]
				]
			]
		+ SVerticalBox::Slot()
		  .VAlign(VAlign_Bottom)
		  .AutoHeight()
		  .Padding(0)
		[
			ErrorReporting->AsWidget()
		]
	];

	float VerticalPaddingAmount = 0.0f;

	// Add padding widgets at the top of the pin boxes if it's a struct operation with a long title
	if (bPadTitle)
	{
		//VerticalPaddingAmount += 16.0f;
	}

	// Add padding to offset the exec pin so that it will align with other nodes
	if (UK2Node* K2Node = Cast<UK2Node>(GraphNode))
	{
		if (!K2Node->IsNodePure())
		{
			//VerticalPaddingAmount += 7.0f;
		}
	}

	if (VerticalPaddingAmount > 0.0f)
	{
		/*	LeftNodeBox->AddSlot()
			           .AutoHeight()
			           .HAlign(HAlign_Left)
			           .VAlign(VAlign_Center)
			[
				SNew(SSpacer).Size(FVector2D(0.0f, VerticalPaddingAmount))
			];
			RightNodeBox->AddSlot()
			            .AutoHeight()
			            .HAlign(HAlign_Left)
			            .VAlign(VAlign_Center)
			[
				SNew(SSpacer).Size(FVector2D(0.0f, VerticalPaddingAmount))
			];*/
	}
	// Create comment bubble
	TSharedPtr<SDefault_CommentBubble> CommentBubble;

	SAssignNew(CommentBubble, SDefault_CommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	// Create widgets for each of the real pins
	CreatePinWidgets();
}

bool SDefault_GraphNodeK2Var::IsInvalid() const
{
	return bVariableColorIsWhite ||
	SDefault_GraphNodeK2Base::IsInvalid();
}

const FSlateBrush* SDefault_GraphNodeK2Var::GetShadowBrush(bool bSelected) const
{
	if (UpdateState(bSelected))
	{
		auto NodeType = GetNodeType();
		const FDTVarNodeState& State = UNodeRestyleSettings::Get()->VarNode.GetTypeData(NodeType).GetState(CachedState);
		auto Body = State.Body.Get();
		CachedOutlineWidth = UDefaultThemeSettings::GetOutlineWidth(Body.OutlineWidth);

		if (TitleTextBlock.IsValid() && TitleTextBlock.ToSharedRef() != SNullWidget::NullWidget)
		{
			StaticCastSharedPtr<STextBlock>(TitleTextBlock)->SetColorAndOpacity(State.TitleColor.Get());
		}
		if (VarNodeBody.IsValid())
		{
			VarNodeBody->SetImage(FAppStyle::Get().GetBrush(FNodeRestyleStyles::VarNode_Body(NodeType, CachedState)));
		}
	}
	return CachedNoDrawBrush;
}

EDTVarType SDefault_GraphNodeK2Var::GetNodeType() const
{
	FLinearColor Color = GetVariableColor().GetSpecifiedColor();
	return EDTVarType::Default;
}
