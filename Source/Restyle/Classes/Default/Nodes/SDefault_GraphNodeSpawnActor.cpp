// Alexander (AgitoReiKen) Moskalenko (C) 2022

#include "SDefault_GraphNodeSpawnActor.h"
#include "KismetNodes/SGraphNodeSpawnActor.h"
#include "Engine/Blueprint.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "K2Node_SpawnActor.h"
#include "KismetPins/SGraphPinObject.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "ScopedTransaction.h"

#include "Default/Pins/SDefault_Pins.h"
#define LOCTEXT_NAMESPACE "SGraphNodeSpawnActor"

//////////////////////////////////////////////////////////////////////////
// SGraphPinSwitchNodeDefaultCaseExec

/**
 * GraphPin can select only actor classes generated by blueprint.
 * Instead of asset picker, a class viewer is used, and blueprint is obtained form class.
 */
class SDefault_GraphPinActorBasedBlueprintClass : public SDefault_GraphPinObject
{
	void OnClassPicked(UClass* InChosenClass)
	{
		AssetPickerAnchor->SetIsOpen(false);

		if (InChosenClass && GraphPinObj)
		{
			check(InChosenClass->ClassGeneratedBy);
			check(InChosenClass->IsChildOf(AActor::StaticClass()));
			if (const UEdGraphSchema* Schema = GraphPinObj->GetSchema())
			{
				const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeClassPinValue", "Change Class Pin Value"));
				GraphPinObj->Modify();

				Schema->TrySetDefaultObject(*GraphPinObj, InChosenClass->ClassGeneratedBy);
			}
		}
	}

	class FActorBasedBlueprintClassFilter : public IClassViewerFilter
	{
	public:

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			if (NULL != InClass)
			{
				const bool bGeneratedByBlueprint = NULL != Cast<UBlueprint>(InClass->ClassGeneratedBy);
				const bool bActorBased = InClass->IsChildOf(AActor::StaticClass());
				return bGeneratedByBlueprint && bActorBased;
			}
			return false;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			const bool bActorBased = InUnloadedClassData->IsChildOf(AActor::StaticClass());
			return bActorBased;
		}
	};

protected:

	virtual FText GetDefaultComboText() const override { return LOCTEXT("DefaultComboText", "Select Blueprint"); }

	virtual TSharedRef<SWidget> GenerateAssetPicker() override
	{
		FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
		//Options.bIsActorsOnly = true;
		//Options.bIsBlueprintBaseOnly = true;
		Options.bShowUnloadedBlueprints = true;
		Options.bShowNoneOption = true;
		Options.bShowObjectRootClass = false;
		TSharedPtr< FActorBasedBlueprintClassFilter > Filter = MakeShareable(new FActorBasedBlueprintClassFilter);
		Options.ClassFilters.Add(Filter.ToSharedRef());

		return
			SNew(SBox)
			.WidthOverride(280)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.MaxHeight(500)
				[
					SNew(SBorder)
					.Padding(4)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					[
						ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SDefault_GraphPinActorBasedBlueprintClass::OnClassPicked))
					]
				]
			];
	}
};

//////////////////////////////////////////////////////////////////////////
// SGraphNodeSpawnActor

void SDefault_GraphNodeSpawnActor::CreatePinWidgets()
{
	UK2Node_SpawnActor* SpawnActorNode = CastChecked<UK2Node_SpawnActor>(GraphNode);
	UEdGraphPin* BlueprintPin = SpawnActorNode->GetBlueprintPin();

	// Create Pin widgets for each of the pins, except for the Blueprint pin
	for (auto PinIt = GraphNode->Pins.CreateConstIterator(); PinIt; ++PinIt)
	{
		UEdGraphPin* CurrentPin = *PinIt;
		if (CurrentPin != BlueprintPin)
		{
			CreateStandardPinWidget(CurrentPin);
		}
	}

	// Handle the Blueprint pin
	if ((BlueprintPin != NULL) && (!BlueprintPin->bHidden || (BlueprintPin->LinkedTo.Num() > 0)))
	{
		TSharedPtr<SGraphPin> NewPin = SNew(SDefault_GraphPinActorBasedBlueprintClass, BlueprintPin);
		check(NewPin.IsValid());

		AddPin(NewPin.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
