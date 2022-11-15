// Alexander (AgitoReiKen) Moskalenko (C) 2022
#include "DefaultGraphPanelNodeFactory.h"

#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_BlendSpaceBase.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "SCommentBubble.h"
#include "SGraphNode.h" 
#include "Editor/BlueprintGraph/Public/BlueprintGraphDefinitions.h"
#include "SGraphPin.h"
#include "K2Node.h"
#include "K2Node_Event.h"
#include "K2Node_AddPinInterface.h"
#include "K2Node_CallMaterialParameterCollectionFunction.h"
#include "K2Node_Composite.h"
#include "K2Node_Copy.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_FormatText.h"
#include "K2Node_Knot.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_SpawnActor.h"
#include "K2Node_SpawnActorFromClass.h"
#include "K2Node_Switch.h"
#include "K2Node_Timeline.h"  
#include "KismetPins/SGraphPinNum.h" 
#include "K2Node_PromotableOperator.h"
#include "MaterialNodes/SGraphNodeMaterialBase.h"
#include "Themes/Default/NodeRestyleDefault.h"
#include "KismetNodes/SGraphNodeK2Var.h"
#include "KismetNodes/SGraphNodeK2Composite.h" 

#include "MaterialGraph/MaterialGraphNode_Comment.h"

#include "Nodes/SDefault_GraphNodeK2Event.h"
#include "Nodes/SDefault_GraphNodeK2Default.h"
#include "Nodes/SDefault_GraphNodeK2Sequence.h"
#include "Nodes/SDefault_GraphNodePromotableOperator.h"
#include "Nodes/SDefault_GraphNodeK2Var.h"
#include "Nodes/SDefault_GraphNodeK2Composite.h"
#include "Nodes/SDefault_GraphNodeKnot.h"
#include "Nodes/SDefault_GraphNodeK2Timeline.h"
#include "Nodes/SDefault_GraphNodeSpawnActor.h"
#include "Nodes/SDefault_GraphNodeSpawnActorFromClass.h"
#include "Nodes/SDefault_GraphNodeFormatText.h"
#include "Nodes/SDefault_GraphNodeK2CreateDelegate.h"
#include "Nodes/SDefault_GraphNodeSwitchStatement.h"
#include "Nodes/SDefault_GraphNodeMakeStruct.h"
#include "Nodes/SDefault_GraphNodeComment.h"
#include "Nodes/SDefault_GraphNodeCreateWidget.h"
#include "UMGEditor/Private/Nodes/K2Node_CreateWidget.h"
#include "Utils/Privates.h"
FDefaultGraphPanelNodeFactory::FDefaultGraphPanelNodeFactory()
{
}

TSharedPtr<SGraphNode> FDefaultGraphPanelNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	if (!InNode) return nullptr;
	auto& Factories = access_private_static::FEdGraphUtilities::VisualNodeFactories();
	for (auto FactoryIt = Factories.CreateIterator(); FactoryIt; ++FactoryIt)
	{
		TSharedPtr<FGraphPanelNodeFactory> FactoryPtr = *FactoryIt;
		if (FactoryPtr.IsValid() && FactoryPtr.Get() != this)
		{
			TSharedPtr<SGraphNode> ResultVisualNode = FactoryPtr->CreateNode(InNode);
			if (ResultVisualNode.IsValid())
			{
				return ResultVisualNode;
			}
		}
	}
	// ReSharper disable once CppJoinDeclarationAndAssignment
	TSharedPtr<SGraphNode> Result;
	Result = TryAnimation(InNode);
	if (Result) return Result;

	Result = TryMaterial(InNode);
	if (Result) return Result;

	Result = TryOther(InNode);
	if (Result) return Result;

	Result = TryKismet(InNode);
	if (Result) return Result;

	return nullptr;
}

TSharedPtr<SGraphNode> FDefaultGraphPanelNodeFactory::TryKismet(UEdGraphNode* InNode) const
{
	if (UK2Node* K2Node = Cast<UK2Node>(InNode))
	{
		if (UK2Node_CreateWidget* CreateWidgetNode = Cast<UK2Node_CreateWidget>(InNode))
		{
			return SNew(SDefault_GraphNodeCreateWidget, CreateWidgetNode);
		}
		if (UK2Node_Composite* CompositeNode = Cast<UK2Node_Composite>(InNode))
		{
			return SNew(SDefault_GraphNodeK2Composite, CompositeNode);
		}
		if (K2Node->DrawNodeAsVariable())
		{
			return SNew(SDefault_GraphNodeK2Var, K2Node);
		}
		if (UK2Node_Switch* SwitchNode = Cast<UK2Node_Switch>(InNode))
		{
			return SNew(SDefault_GraphNodeSwitchStatement, SwitchNode);
		}
		if (UK2Node_PromotableOperator* PromotableOperator = Cast<UK2Node_PromotableOperator>(InNode))
		{
			return SNew(SDefault_GraphNodePromotableOperator, PromotableOperator);
		}
		if (InNode->GetClass()->ImplementsInterface(UK2Node_AddPinInterface::StaticClass()))
		{
			return SNew(SDefault_GraphNodeK2Sequence, CastChecked<UK2Node>(InNode));
		}
		if (UK2Node_Timeline* TimelineNode = Cast<UK2Node_Timeline>(InNode))
		{
			return SNew(SDefault_GraphNodeK2Timeline, TimelineNode);
		}
		if (UK2Node_SpawnActor* SpawnActorNode = Cast<UK2Node_SpawnActor>(InNode))
		{
			return SNew(SDefault_GraphNodeSpawnActor, SpawnActorNode);
		}
		if (UK2Node_SpawnActorFromClass* SpawnActorNodeFromClass = Cast<UK2Node_SpawnActorFromClass>(InNode))
		{
			return SNew(SDefault_GraphNodeSpawnActorFromClass, SpawnActorNodeFromClass);
		}
		if (UK2Node_CreateDelegate* CreateDelegateNode = Cast<UK2Node_CreateDelegate>(InNode))
		{
			return SNew(SDefault_GraphNodeK2CreateDelegate, CreateDelegateNode);
		}
		if (UK2Node_CallMaterialParameterCollectionFunction* CallFunctionNode = Cast<UK2Node_CallMaterialParameterCollectionFunction>(InNode))
		{
			return nullptr;
			//return SNew(SGraphNodeCallParameterCollectionFunction, CallFunctionNode);
		}
		if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(InNode))
		{
			return SNew(SDefault_GraphNodeK2Event, EventNode);
		}
		if (UK2Node_FormatText* FormatTextNode = Cast<UK2Node_FormatText>(InNode))
		{
			return SNew(SDefault_GraphNodeFormatText, FormatTextNode);
		}
		if (UK2Node_Knot* Knot = Cast<UK2Node_Knot>(InNode))
		{
			return SNew(SDefault_GraphNodeKnot, Knot);
		}
		if (UK2Node_MakeStruct* MakeStruct = Cast<UK2Node_MakeStruct>(InNode))
		{
			return SNew(SDefault_GraphNodeMakeStruct, MakeStruct);
		}
		if (UK2Node_Copy* CopyNode = Cast<UK2Node_Copy>(InNode))
		{
			return nullptr;
			//return SNew(SGraphNodeK2Copy, CopyNode);
		}
		return SNew(SDefault_GraphNodeK2Default, K2Node);
	}
	return nullptr;
}

TSharedPtr<SGraphNode> FDefaultGraphPanelNodeFactory::TryAnimation(UEdGraphNode* InNode) const
{
	//if (UAnimGraphNode_Base* BaseAnimNode = Cast<UAnimGraphNode_Base>(InNode))
	//{
	//	if (UAnimGraphNode_Root* RootAnimNode = Cast<UAnimGraphNode_Root>(InNode))
	//	{
	//		return nullptr;
	//		//return SNew(SGraphNodeAnimationResult, RootAnimNode);
	//	}
	//	else if (UAnimGraphNode_StateMachineBase* StateMachineInstance = Cast<UAnimGraphNode_StateMachineBase>(InNode))
	//	{
	//		return SNew(SGraphNodeStateMachineInstance, StateMachineInstance);
	//	}
	//	else if (UAnimGraphNode_SequencePlayer* SequencePlayer = Cast<UAnimGraphNode_SequencePlayer>(InNode))
	//	{
	//		return SNew(SGraphNodeSequencePlayer, SequencePlayer);
	//	}
	//	else if (UAnimGraphNode_LayeredBoneBlend* LayeredBlend = Cast<UAnimGraphNode_LayeredBoneBlend>(InNode))
	//	{
	//		return SNew(SGraphNodeLayeredBoneBlend, LayeredBlend);
	//	}
	//	else if (UAnimGraphNode_BlendSpaceBase* BlendSpacePlayer = Cast<UAnimGraphNode_BlendSpaceBase>(InNode))
	//	{
	//		return SNew(SGraphNodeBlendSpacePlayer, BlendSpacePlayer);
	//	}
	//	else if (UAnimGraphNode_BlendSpaceGraphBase* BlendSpaceGraph = Cast<UAnimGraphNode_BlendSpaceGraphBase>(InNode))
	//	{
	//		return SNew(SGraphNodeBlendSpaceGraph, BlendSpaceGraph);
	//	}
	//	else
	//	{
	//		return SNew(SAnimationGraphNode, BaseAnimNode);
	//	}
	//}
	//else if (UAnimStateTransitionNode* TransitionNode = Cast<UAnimStateTransitionNode>(InNode))
	//{
	//	return SNew(SGraphNodeAnimTransition, TransitionNode);
	//}
	//else if (UAnimStateNode* StateNode = Cast<UAnimStateNode>(InNode))
	//{
	//	return SNew(SGraphNodeAnimState, StateNode);
	//}
	//else if (UAnimStateAliasNode* StateAliasNode = Cast<UAnimStateAliasNode>(InNode))
	//{
	//	return SNew(SGraphNodeAnimStateAlias, StateAliasNode);
	//}
	//else if (UAnimStateConduitNode* ConduitNode = Cast<UAnimStateConduitNode>(InNode))
	//{
	//	return SNew(SGraphNodeAnimConduit, ConduitNode);
	//}
	//else if (UAnimStateEntryNode* EntryNode = Cast<UAnimStateEntryNode>(InNode))
	//{
	//	return SNew(SGraphNodeAnimStateEntry, EntryNode);
	//}
	//else if (UK2Node_AnimNodeReference* AnimNodeReference = Cast<UK2Node_AnimNodeReference>(InNode))
	//{
	//	return SNew(SAnimNodeReference, AnimNodeReference);
	//}
	return nullptr;
}

TSharedPtr<SGraphNode> FDefaultGraphPanelNodeFactory::TryMaterial(UEdGraphNode* InNode) const
{
	if (UMaterialGraphNode_Comment* MaterialCommentNode = Cast<UMaterialGraphNode_Comment>(InNode))
	{
		return nullptr;
		//return SNew(SGraphNodeMaterialComment, MaterialCommentNode);
	}
	return nullptr;
}

TSharedPtr<SGraphNode> FDefaultGraphPanelNodeFactory::TryOther(UEdGraphNode* InNode) const
{
	if (UEdGraphNode_Comment* CommentNode = Cast<UEdGraphNode_Comment>(InNode))
	{
		return SNew(SDefault_GraphNodeComment, CommentNode);
	}
	return nullptr;
}
